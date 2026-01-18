#include <ShaderUtils/ShaderUtils.hpp>

#include <fstream>
#include <filesystem>
#include <ranges>
#include <array>

namespace ShaderCompiler::ShaderUtils
{
        namespace {

            /**
             * @brief Token for find header declaration.
             */
            constexpr std::string_view header_token{"#include"};

            /**
             * @brief UTF8 file's mark.
             */
            constexpr std::array BOM{0xEF, 0xBB, 0xBF};

            /**
             * @brief Reads file and returns this one's content via str.
             *
             * @param file_name File path.
             * @return File content.
             */
            [[nodiscard]]
            std::string read_file(std::string_view file_name) noexcept(false) {
                std::ifstream file{file_name.data(), std::ios::in};

                file.exceptions(std::ios::failbit | std::ios::badbit);
                if (!file) {
                    throw std::runtime_error("Can't open file: {}" + std::string{file_name});
                }

                std::string content{
                    std::istreambuf_iterator<char>(file),
                    std::istreambuf_iterator<char>()};
                file.close();
                return content;
            }

            /**
             * @brief Replaces all @c #include directives in the input source code with the contents of the corresponding
             * header files.
             *
             * This function parses the given source code, searches for @ #include directives, and replaces them
             * with the actual content of the respective header files. The search for the header files is conducted
             * within the directories specified in the @c include_dirs parameter. If any @c #include directive
             * references a file that cannot be found or there are formatting issues with the directive, exceptions
             * are thrown.
             *
             * @param src_code The input source code.
             * @param include_dirs A list of directories to search for the header files referenced in the @c #include
             * directives.
             * @return The formatted source code.
             */
            [[nodiscard]]
            std::string replace_headers(const std::string_view src_code, std::span<const std::string> include_dirs)
                    noexcept(false) {
                std::string out{src_code};

                auto currentIt{std::begin(out)};

                do {
                    auto tokenStart = std::ranges::search(
                        currentIt, std::end(out),
                        std::begin(header_token), std::end(header_token));

                    if (std::begin(tokenStart) == std::end(out)) {
                        break;
                    }

                    const auto filenameStart = std::ranges::find(std::begin(tokenStart), std::end(out), '<');
                    const auto filenameEnd = std::ranges::find(filenameStart, std::end(out), '>');

                    if (filenameStart == std::end(out) || filenameEnd == std::end(out)) {
                        throw std::runtime_error(std::format("Error on parsing include token: %s\n", src_code.data()));
                    }

                    std::filesystem::path filename{filenameStart + 1, filenameEnd};

                    const auto headerPath = std::ranges::find_if(include_dirs, [&](const auto& dir) {
                        return std::filesystem::exists(dir / filename);
                    });

                    if (headerPath == std::end(include_dirs)) {
                        throw std::runtime_error(std::format("Error on searching header file: %s\n", filename.string()));
                    }
                    filename = *headerPath / filename;
                    const auto headerContent{read_file(filename.string())};

                    currentIt = out.erase(std::begin(tokenStart), filenameEnd + 1);
                    auto insertPos{out.insert(currentIt, std::begin(headerContent), std::end(headerContent))};
                    currentIt = insertPos + std::size(headerContent);
                } while (currentIt != std::end(out));

                return out;
            }
        }

	int endsWith(const char* s, const char* part)
	{
		return (strstr(s, part) - s) == (strlen(s) - strlen(part));
	}

	std::string readShaderFile(const std::string_view fileName, const std::span<const std::string> includeDirs)
	{
                /// 1. Reads shader from file
                std::string sourceCode;
                try {
                    sourceCode = read_file(fileName);
                } catch (const std::exception& e) {
                    std::printf("Error on read \"%s\" file reading: %s\n", fileName.data(), e.what());
                    return {};
                }

                /// 2. Checks it starts with BOM
                if (std::size(sourceCode) < std::size(BOM)) {
                    return sourceCode;
                }

                const auto startWithBom = std::ranges::equal(
                    std::begin(sourceCode), std::begin(sourceCode) + 3,
                    std::begin(BOM), std::end(BOM));

                if (startWithBom) {
                    std::fill_n(std::begin(sourceCode), std::size(BOM), ' ');
                }

                /// 3. Replaces all include directives
                std::string resCode;
                try {
                    resCode = replace_headers(sourceCode, includeDirs);
                } catch (const std::exception& e) {
                    std::printf("Error on loading shader program: %s\n", e.what());
                    return {};
                }

                return resCode;
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
