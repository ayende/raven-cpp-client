#include "stdafx.h"
#include "CurlResponse.h"
#include "CurlSListHolder.h"

namespace
{
	void left_trim(std::string &s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
			return !std::isspace(ch);
		}));
	}

	void right_trim(std::string &s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
			return !std::isspace(ch);
		}).base(), s.end());
	}

	size_t push_headers(char *buffer, size_t size, size_t nitems, void *userdata)
	{
		const auto real_size = size * nitems;
		auto headers = static_cast<std::unordered_map<std::string, std::string>*>(userdata);
		auto delimiter = static_cast<char*>(memchr(buffer, ':', real_size));
		if (delimiter == nullptr) // doesn't have ':', probably not a header line, not interesting
			return real_size;

		auto header_name_size = delimiter - buffer;

		auto header_name = std::string(buffer, header_name_size);

		auto header_val = std::string(delimiter + 1, real_size - header_name_size - 1);
		// remove starting space and \r\n at end
		left_trim(header_val);
		right_trim(header_val);

		headers->emplace(header_name, header_val);

		return real_size;
	}

	size_t push_to_buffer(char* contents, size_t size, size_t nmemb, void* output_buffer_void)
	{
		auto real_size = size * nmemb;
		auto& output_buffer = *static_cast<std::string*>(output_buffer_void);
		output_buffer.append(contents, real_size);
		return real_size;
	}

}

namespace ravendb::client::impl
{
	CurlResponse CurlResponse::run_curl_perform(const CurlHandlesHolder::CurlReference& curl_ref)
	{
		return CurlResponse(curl_ref);
	}

	bool CurlResponse::is_valid() const
	{
		return _valid;
	}

	CurlResponse::CurlResponse(const CurlHandlesHolder::CurlReference& curl_ref)
	{
		CURL* curl = curl_ref.get();
		CurlSListHolder headers_list{};

		for(const auto&[name, value] : curl_ref.headers)
		{
			std::ostringstream header{};
			header << name << ": " << value;
			headers_list.append(header.str());
		}
		curl_easy_setopt(curl, CURLOPT_URL, curl_ref.url.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers_list.get());
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, _error_buffer);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, push_to_buffer);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, push_headers);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headers);

		if(curl_ref.set_before_perform && curl_ref.set_before_perform.value())
		{
			(*curl_ref.set_before_perform)(curl);
		}

		perform_result = curl_easy_perform(curl);

		error = _error_buffer;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, nullptr);

		if(curl_ref.set_after_perform && curl_ref.set_after_perform.value())
		{
			(*curl_ref.set_after_perform)(curl);
		}
	}
}