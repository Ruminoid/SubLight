#include "Sublight.Charset.h"
#include <memory>
#include <uchardet.h>
#include <utility>
#include <iconv.h>
#include <functional>

/// @return pair (encoding name, skipping_bytes)
std::pair<std::string, size_t> DetectEncoding(std::string_view input)
{
	auto data = input.data();
	auto size = input.size();
	if (size >= 3 && !strncmp(data, "\xef\xbb\xbf", 3))
		return std::make_pair(std::string("UTF-8"), 3);
	if (size >= 4 && !strncmp(data, "\x00\x00\xfe\xff", 4))
		return std::make_pair(std::string("UTF-32BE"), 4);
	if (size >= 4 && !strncmp(data, "\xff\xfe\x00\x00", 4))
		return std::make_pair(std::string("UTF-32BE"), 4);
	if (size >= 2 && !strncmp(data, "\xfe\xff", 2))
		return std::make_pair(std::string("UTF-16BE"), 2);
	if (size >= 2 && !strncmp(data, "\xff\xfe", 2))
		return std::make_pair(std::string("UTF-16BE"), 2);

    auto ud = uchardet_new();
	uchardet_handle_data(ud, data, size);
	uchardet_data_end(ud);
	auto result = std::make_pair(std::string(uchardet_get_charset(ud)), 0);
    uchardet_delete(ud);
    return result;
}

std::pair<char*, size_t> ConvertToUTF8(char* data, size_t size, const char* codepage)
{
    iconv_t ic;
    char* tempOut;

    if ((ic = iconv_open("UTF-8", codepage)) == (iconv_t)(-1))
        return std::make_pair(nullptr, 0);

    auto icHandle = &ic;
    std::unique_ptr<iconv_t, std::function<void(iconv_t*)>> icGuard(icHandle, [](iconv_t* ptr)
    {
        if (*ptr != (iconv_t)(-1)) {
            (void)iconv_close(*ptr);
        }
    });

    size_t osize = size;
    size_t ileft = size;
    size_t oleft = size - 1;
    char* ip;
    char* op;
    size_t rc;
    int clear = 0;

    tempOut = reinterpret_cast<char*>(malloc(osize));
    std::unique_ptr<char, decltype(&free)> tempOutGuard(tempOut, &free);
    if (!tempOut)
        return std::make_pair(nullptr, 0);
    ip = data;
    op = tempOut;

    while (true) {
        if (ileft)
            rc = iconv(ic, &ip, &ileft, &op, &oleft);
        else {              // clear the conversion state and leave
            clear = 1;
            rc = iconv(ic, NULL, NULL, &op, &oleft);
        }
        if (rc == static_cast<size_t>(-1)) {
            if (errno == E2BIG) {
                size_t offset = op - tempOut;
                auto nbuf = reinterpret_cast<char*>(realloc(tempOut, osize + size));
                if (!nbuf) {
                    return std::make_pair(nullptr, 0);
                }
                tempOut = nbuf;
                tempOutGuard.release();
                tempOutGuard.reset(tempOut);
                op = tempOut + offset;
                osize += size;
                oleft += size;
            }
            else {
                return std::make_pair(nullptr, 0);
            }
        }
        else if (clear)
            break;
    }
    auto actualSize = osize - oleft;
    auto output = new char[actualSize];
    memcpy_s(output, actualSize, tempOut, actualSize);
    return std::make_pair(output, actualSize);
}

std::pair<char*, size_t> ConvertToUTF8(std::string_view input)
{
	auto size = input.size();
	auto encodingPair = DetectEncoding(input);
    if (encodingPair.first == "")
    {
        auto data = new char[size];
        memcpy_s(data, size, input.data(), size);
        return std::make_pair(data, size);
    }

	auto encodingName = encodingPair.first;
	auto skipping = encodingPair.second;
	auto newSize = size - skipping;
	if (encodingName == "UTF-8")
	{
		auto data = new char[newSize];
		memcpy_s(data, newSize, input.data() + skipping, newSize);
		return std::make_pair(data, size - skipping);
	}

    return ConvertToUTF8(const_cast<char*>(input.data() + skipping), size, encodingPair.first.c_str());
}
