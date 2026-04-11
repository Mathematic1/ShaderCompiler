#include <ShaderUtils/ShaderUtils.hpp>

#include <fstream>
#include <filesystem>
#include <ranges>
#include <array>
#include <vector>
#include <format>
#include <optional>

namespace ShaderCompiler::ShaderUtils
{
        namespace {

            /**
             * @brief Token for find header declaration.
             */
            constexpr std::string_view headerToken{"#include"};

            /**
             * @brief UTF8 file's mark.
             */
            constexpr std::array BOM{0xEF, 0xBB, 0xBF};

            constexpr auto isWhitespace = [](char c) { return c == ' ' || c == '\t'; };
            constexpr auto isNewline    = [](char c) { return c == '\n' || c == '\r'; };

            struct IncludeDirective {
                std::filesystem::path name;
                std::string::iterator tokenBegin;
                std::string::iterator eraseEnd;
            };

            /**
             * @brief Reads file and returns this one's content via str.
             *
             * @param file_name File path.
             * @return File content.
             */
            [[nodiscard]]
            std::string readFile(std::string_view fileName) {
                std::ifstream file{fileName.data(), std::ios::in};
                file.exceptions(std::ios::failbit | std::ios::badbit);
                if (!file) {
                    throw std::runtime_error(std::format("Can't open file: {}", fileName));
                }

                return {std::istreambuf_iterator<char>(file),
                        std::istreambuf_iterator<char>()};
            }

            [[nodiscard]]
            std::filesystem::path findHeaderFile(
                const std::filesystem::path& includeName,
                std::span<const std::string> includeDirs,
                const std::filesystem::path& currentFileDir)
            {
                if (auto it = std::ranges::find_if(includeDirs, [&](const auto& dir) {
                        return std::filesystem::exists(std::filesystem::path(dir) / includeName);
                    }); it != includeDirs.end())
                {
                    return std::filesystem::path(*it) / includeName;
                }

                for (auto dir = currentFileDir; !dir.empty(); ) {
                    if (auto candidate = dir / includeName; std::filesystem::exists(candidate)) {
                        return candidate;
                    }
                    auto parent = dir.parent_path();
                    if (parent == dir) break;
                    dir = std::move(parent);
                }

                throw std::runtime_error(std::format("Error on searching header file: {}", includeName.string()));
            }

            [[nodiscard]]
            std::optional<IncludeDirective> tryParseIncludeOnLine(
                std::string& out,
                std::string::iterator searchFrom)
            {
                const auto tokenMatch = std::ranges::search(
                    searchFrom, out.end(),
                    headerToken.begin(), headerToken.end()
                );

                if (tokenMatch.begin() == out.end()) {
                    return std::nullopt;
                }

                const auto lineEnd = std::find(tokenMatch.begin(), out.end(), '\n');
                const auto nextLine = (lineEnd == out.end()) ? out.end() : std::next(lineEnd);

                auto makeSkip = [&]() {
                    return IncludeDirective{.name = {}, .tokenBegin = nextLine, .eraseEnd = nextLine};
                };

                // Ensure only whitespace between line start and #include
                const auto lineStart = std::find_if(
                    std::make_reverse_iterator(tokenMatch.begin()),
                    std::make_reverse_iterator(out.begin()),
                    isNewline
                ).base();

                if (!std::ranges::all_of(lineStart, tokenMatch.begin(), isWhitespace)) {
                    return makeSkip();
                }

                // Skip whitespace after "#include"
                auto p = std::find_if_not(tokenMatch.begin() + std::ssize(headerToken), lineEnd, isWhitespace);

                if (p == lineEnd || (*p != '<' && *p != '"')) {
                    return makeSkip();
                }

                const char close = (*p == '<') ? '>' : '"';
                const auto fileBegin = std::next(p);
                const auto fileEnd = std::find(fileBegin, lineEnd, close);
                if (fileEnd == lineEnd) {
                    throw std::runtime_error("Error on parsing include token in line");
                }

                return IncludeDirective{
                    .name = std::filesystem::path(fileBegin, fileEnd),
                    .tokenBegin = tokenMatch.begin(),
                    .eraseEnd = std::next(fileEnd)
                };
            }

            [[nodiscard]]
            std::string replaceHeaders(
                std::string_view srcCode,
                std::span<const std::string> includeDirs,
                std::vector<std::filesystem::path>& includeStack,
                const std::filesystem::path& currentFileDir)
            {
                std::string out{srcCode};
                auto currentIt = out.begin();

                while (currentIt != out.end()) {
                    auto parsed = tryParseIncludeOnLine(out, currentIt);
                    if (!parsed) break;

                    if (parsed->name.empty()) {
                        currentIt = parsed->tokenBegin;
                        continue;
                    }

                    const auto fullPath = findHeaderFile(parsed->name, includeDirs, currentFileDir).lexically_normal();

                    if (std::ranges::find(includeStack, fullPath) != includeStack.end()) {
                        throw std::runtime_error(std::format("Cyclic include detected: {}", fullPath.string()));
                    }

                    includeStack.push_back(fullPath);
                    const auto expanded = replaceHeaders(
                        readFile(fullPath.string()), includeDirs, includeStack, fullPath.parent_path()
                    );
                    includeStack.pop_back();

                    currentIt = out.erase(parsed->tokenBegin, parsed->eraseEnd);
                    auto insertPos = out.insert(currentIt, expanded.begin(), expanded.end());
                    currentIt = insertPos + static_cast<std::ptrdiff_t>(expanded.size());
                }

                return out;
            }

            [[nodiscard]]
            std::string replaceHeaders(
                std::string_view srcCode,
                std::span<const std::string> includeDirs,
                const std::filesystem::path& currentFileDir)
            {
                std::vector<std::filesystem::path> includeStack;
                return replaceHeaders(srcCode, includeDirs, includeStack, currentFileDir);
            }
        }

	bool endsWith(const char* s, const char* part)
	{
            return std::string_view{s}.ends_with(part);
	}

	std::string readShaderFile(const std::string_view fileName, const std::span<const std::string> includeDirs)
	{
            /// 1. Read shader source
            std::string sourceCode;
            try {
                sourceCode = readFile(fileName);
            } catch (const std::exception& e) {
                std::printf("Error on read \"%s\" file reading: %s\n", fileName.data(), e.what());
                return {};
            }

            /// 2. Strip BOM if present
            if (sourceCode.size() >= BOM.size()) {
                if (std::ranges::equal(
                        sourceCode.begin(), sourceCode.begin() + std::ssize(BOM),
                        BOM.begin(), BOM.end()))
                {
                    std::fill_n(sourceCode.begin(), BOM.size(), ' ');
                }
            }

            /// 3. Expand all #include directives
            try {
                return replaceHeaders(sourceCode, includeDirs,
                                      std::filesystem::path(fileName).parent_path());
            } catch (const std::exception& e) {
                std::printf("Error on loading shader program: %s\n", e.what());
                return {};
            }
	}

	void printShaderSource(const char* text)
	{
		int line = 1;
		printf("\n(%3i) ", line);
		while (text && *text++)
		{
			if (*text == '\n') printf("\n(%3i) ", ++line);
			else if (*text == '\r') {}
			else printf("%c", *text);
		}
		printf("\n");
	}
}
