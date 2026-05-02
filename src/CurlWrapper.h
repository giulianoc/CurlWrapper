/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   CURL.h
 *
 * Created on March 29, 2018, 6:27 AM
 */

#pragma once

#include <stdexcept>
#include <string_view>
#include "JSONUtils.h"
#include "spdlog/spdlog.h"
#include <curl/curl.h>
#include <deque>
#include <fstream>
#include <vector>


class CurlWrapper
{
  public:
	struct CurlException : public std::runtime_error
	{
		explicit CurlException(const std::string &message) : runtime_error(message) {};
		~CurlException() noexcept override = default;

		[[nodiscard]] virtual std::string_view type() const noexcept { return "CurlException"; }
	};

	struct ServerNotReachable final : public CurlException
	{
		explicit ServerNotReachable(const std::string &message) : CurlException(message) {};
		~ServerNotReachable() noexcept override = default;

		[[nodiscard]] std::string_view type() const noexcept override { return "ServerNotReachable"; }
	};

	struct HTTPError final : public CurlException
	{
		int16_t httpErrorCode;
		HTTPError(int16_t httpErrorCode, const std::string &message) : CurlException(message), httpErrorCode(httpErrorCode) {};
		~HTTPError() noexcept override = default;

		[[nodiscard]] std::string_view type() const noexcept override { return "HTTPError"; }
	};

	struct UploadChunksInterruptedByUser final : std::runtime_error
	{
		size_t chunkIndex;
		size_t chunksNumber;
		UploadChunksInterruptedByUser(size_t chunkIndex, size_t chunksNumber, const std::string &message) : runtime_error(message),
			chunkIndex(chunkIndex), chunksNumber(chunksNumber) {};
		~UploadChunksInterruptedByUser() noexcept override = default;
	};

	struct CurlDownloadData
	{
		std::string referenceToLog;
		int currentChunkNumber;
		std::string destBinaryPathName;
		std::ofstream mediaSourceFileStream;
		size_t currentTotalSize;
		size_t maxChunkFileSize;
	};

	struct CurlUploadData
	{
		std::ifstream mediaSourceFileStream;

		int64_t payloadBytesSent;
		int64_t upToByte_Excluded;
	};

	struct CurlUploadFormData
	{
		std::ifstream mediaSourceFileStream;

		int64_t payloadBytesSent;
		int64_t upToByte_Excluded;

		bool formDataSent;
		std::string formData;

		bool endOfFormDataSent;
		std::string endOfFormData;
	};

	struct CurlUploadEmailData
	{
		std::deque<std::string> emailLines;
	};

	// usato nel caso in cui verbose è attivato
	struct CurlDebugContext
	{
		bool enabled = false;
		std::string trace;   // qui accumuliamo il verbose
	};

	struct GetInputParameters
	{
		std::string url;
		long timeoutInSeconds;
		std::string authorization;
		std::vector<std::string> otherHeaders;
		std::string referenceToLog;
		int maxRetryNumber = 0;
		int secondsToWaitBeforeToRetry = 15;
		bool outputCompressed = false;
		std::optional<std::string> proxyURL = std::nullopt;
		std::optional<std::string> proxyUsername = std::nullopt;
		std::optional<std::string> proxyPassword = std::nullopt;
		std::optional<std::string> httpSSLVersion = std::nullopt;
		bool verbose = false;
	};

	struct OutputParameters
	{
		// entrambi i campi sono inizializzati
		std::vector<uint8_t> binary;
		std::string response;

		std::vector<std::pair<std::string, std::string>> responseHeaders;

		std::string getResponseHeaderValue(const std::string& key, const bool emptyIfNotFound = true)
		{
			for (const auto& [headerKey, headerValue] : responseHeaders)
			{
				if (key == headerKey)
					return headerValue;
			}
			if (emptyIfNotFound)
				return "";

			std::string errorMessage = std::format("Response header {} not found", key);
			LOG_ERROR(errorMessage);
			throw std::runtime_error(errorMessage);
		}
	};

	static void globalInitialize();

	static void globalTerminate();

	static std::string escape(const std::string &url);
	static std::string unescape(const std::string &url);

	static std::string basicAuthorization(const std::string &user, const std::string &password);
	static std::string bearerAuthorization(const std::string &bearerToken);

	static void httpGetBinary(GetInputParameters& inputParameters, OutputParameters& outputParameters);

	static std::string httpGet(GetInputParameters& inputParameters, OutputParameters& outputParameters);

	static nlohmann::json httpGetJson(GetInputParameters& inputParameters, OutputParameters& outputParameters);

	static std::string httpDelete(
		std::string url, long timeoutInSeconds, std::string authorization,
		const std::vector<std::string>& otherHeaders = std::vector<std::string>(), std::string referenceToLog = "",
		int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static std::string httpPostString(
		const std::string& url, long timeoutInSeconds, const std::string& authorization, const std::string& body,
		const std::string& contentType, // i.e.: application/json
		const std::vector<std::string>& otherHeaders, const std::string& referenceToLog, int maxRetryNumber, int secondsToWaitBeforeToRetry,
		bool outputCompressed, const std::optional<std::string> &proxyURL = std::nullopt,
		const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static std::string httpPutString(
		const std::string& url, long timeoutInSeconds, const std::string& authorization, const std::string &body,
		const std::string& contentType, // i.e.: application/json
		const std::vector<std::string>& otherHeaders, const std::string& referenceToLog, int maxRetryNumber, int secondsToWaitBeforeToRetry,
		bool outputCompressed,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static std::pair<std::string, std::string> httpPostString(
		const std::string &url, long timeoutInSeconds, const std::string &authorization, const std::string& body, const std::string &contentType = "application/json",
		const std::vector<std::string>& otherHeaders = std::vector<std::string>(), const std::string &referenceToLog = "", int maxRetryNumber = 0,
		int secondsToWaitBeforeToRetry = 15,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static std::pair<std::string, std::string> httpPutString(
		const std::string &url, long timeoutInSeconds, const std::string &authorization, const std::string& body,
		const std::string &contentType = "application/json", const std::vector<std::string>& otherHeaders = std::vector<std::string>(),
		const std::string &referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static nlohmann::json httpPostStringAndGetJson(
		const std::string& url, long timeoutInSeconds, const std::string &authorization, const std::string &body, const std::string& contentType = "application/json",
		const std::vector<std::string>& otherHeaders = std::vector<std::string>(), const std::string& referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15,
		bool outputCompressed = false,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static nlohmann::json httpPutStringAndGetJson(
		const std::string& url, long timeoutInSeconds, const std::string &authorization, const std::string &body, const std::string& contentType = "application/json",
		const std::vector<std::string>& otherHeaders = std::vector<std::string>(), const std::string& referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15,
		bool outputCompressed = false,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static std::string httpPostFile(
		const std::string& url, long timeoutInSeconds, const std::string& authorization, const std::string& pathFileName,
		uintmax_t fileSizeInBytes, const std::string& contentType = "",
		const std::string& referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15, int64_t contentRangeStart = -1,
		int64_t contentRangeEnd_Excluded = -1,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static std::string httpPutFile(
		std::string url, long timeoutInSeconds, const std::string &authorization, const std::string& pathFileName, int64_t fileSizeInBytes, const std::string& contentType = "",
		const std::string& referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15, int64_t contentRangeStart = -1,
		int64_t contentRangeEnd_Excluded = -1,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static nlohmann::json httpPostFileAndGetJson(
		std::string url, long timeoutInSeconds, const std::string& authorization, const std::string &pathFileName, int64_t fileSizeInBytes,
		const std::string& referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15,
		int64_t contentRangeStart = -1, int64_t contentRangeEnd_Excluded = -1,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static nlohmann::json httpPutFileAndGetJson(
		const std::string &url, long timeoutInSeconds, const std::string& authorization, const std::string& pathFileName,
		int64_t fileSizeInBytes, const std::string& referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15,
		int64_t contentRangeStart = -1, int64_t contentRangeEnd_Excluded = -1,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static std::string httpPostFileSplittingInChunks(
		const std::string& url, long timeoutInSeconds, const std::string& authorization, const std::string &pathFileName,
		const std::function<bool(int, int)>& chunkCompleted, const std::string& referenceToLog = "", int maxRetryNumber = 0,
		int secondsToWaitBeforeToRetry = 15,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static std::string httpPostFormData(
		const std::string &url, const std::vector<std::pair<std::string, std::string>> &formData, long timeoutInSeconds, const std::string &referenceToLog = "", int maxRetryNumber = 0,
		int secondsToWaitBeforeToRetry = 15,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static std::string httpPutFormData(
		const std::string &url, const std::vector<std::pair<std::string, std::string>> &formData, long timeoutInSeconds, const std::string &referenceToLog = "", int maxRetryNumber = 0,
		int secondsToWaitBeforeToRetry = 15,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static nlohmann::json httpPostFormDataAndGetJson(
		const std::string &url, const std::vector<std::pair<std::string, std::string>>& formData, long timeoutInSeconds, const std::string& referenceToLog = "", int maxRetryNumber = 0,
		int secondsToWaitBeforeToRetry = 15,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static nlohmann::json httpPutFormDataAndGetJson(
		const std::string &url, const std::vector<std::pair<std::string, std::string>>& formData, long timeoutInSeconds, const std::string &referenceToLog = "", int maxRetryNumber = 0,
		int secondsToWaitBeforeToRetry = 15,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static std::string httpPostFileByFormData(
		const std::string &url, const std::vector<std::pair<std::string, std::string>> &formData, long timeoutInSeconds, const std::string &pathFileName, int64_t fileSizeInBytes,
		const std::string &mediaContentType, const std::string &referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15,
		int64_t contentRangeStart = -1, int64_t contentRangeEnd_Excluded = -1,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static std::string httpPutFileByFormData(
		const std::string &url, const std::vector<std::pair<std::string, std::string>> &formData, long timeoutInSeconds, const std::string &pathFileName, int64_t fileSizeInBytes,
		const std::string &mediaContentType, const std::string &referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15,
		int64_t contentRangeStart = -1, int64_t contentRangeEnd_Excluded = -1,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static nlohmann::json httpPostFileByFormDataAndGetJson(
		const std::string& url, const std::vector<std::pair<std::string, std::string>> &formData, long timeoutInSeconds, const std::string& pathFileName, int64_t fileSizeInBytes,
		const std::string& mediaContentType, const std::string& referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15,
		int64_t contentRangeStart = -1, int64_t contentRangeEnd_Excluded = -1,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static nlohmann::json httpPutFileByFormDataAndGetJson(
		const std::string& url, const std::vector<std::pair<std::string, std::string>>& formData, long timeoutInSeconds, const std::string &pathFileName, int64_t fileSizeInBytes,
		const std::string& mediaContentType, const std::string& referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15,
		int64_t contentRangeStart = -1, int64_t contentRangeEnd_Excluded = -1,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static void downloadFile(
		std::string url, std::string destBinaryPathName, int (*progressCallback)(void *, curl_off_t, curl_off_t, curl_off_t, curl_off_t),
		void *progressData, long downloadChunkSizeInMegaBytes = 500, std::string referenceToLog = "", std::optional<long> timeoutInSeconds = 120,
		int maxRetryNumber = 0, bool resumeActive = false, int secondsToWaitBeforeToRetry = 15,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt, const std::optional<std::string> &httpSSLVersion = std::nullopt,
		bool verbose = false
	);

	static void ftpFile(
		std::string filePathName, const std::string& fileName, int64_t sizeInBytes, std::string ftpServer, int ftpPort, std::string ftpUserName, std::string ftpPassword,
		std::string ftpRemoteDirectory, const std::string& ftpRemoteFileName, int (*progressCallback)(void *, curl_off_t, curl_off_t, curl_off_t, curl_off_t),
		void *progressData, std::string referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static void sendEmail(
		std::string emailServerURL, const std::string& useName, const std::string& password, std::string from, std::string tosCommaSeparated, std::string ccsCommaSeparated, std::string subject,
		std::vector<std::string> &emailBody, std::string contentType,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

  private:
	static std::pair<std::string, std::string> httpPostPutString(
		std::string url, const std::string &requestType,
		// POST or PUT
		long timeoutInSeconds, std::string authorization, const std::string &body, std::string contentType,
		// i.e.: application/json
		const std::vector<std::string> &otherHeaders, std::string referenceToLog, int maxRetryNumber, int secondsToWaitBeforeToRetry,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static void httpPostPutBinary(
		const std::string& url,
		const std::string& requestType, // POST or PUT
		long timeoutInSeconds, const std::string& authorization, const std::string& body,
		const std::string& contentType, // i.e.: application/json
		const std::vector<std::string>& otherHeaders, const std::string& referenceToLog, int maxRetryNumber,
		int secondsToWaitBeforeToRetry, std::vector<uint8_t> &binary,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static std::string httpPostPutFile(
		const std::string& url,
		const std::string& requestType, // POST or PUT
		long timeoutInSeconds, const std::string& authorization, const std::string& pathFileName, int64_t fileSizeInBytes,
		const std::string& contentType, const std::string& referenceToLog,
		int maxRetryNumber, int secondsToWaitBeforeToRetry, int64_t contentRangeStart, int64_t contentRangeEnd_Excluded,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static std::string httpPostPutFormData(
		std::string url, const std::vector<std::pair<std::string, std::string>>& formData,
		const std::string& requestType, // POST or PUT
		long timeoutInSeconds, std::string referenceToLog, int maxRetryNumber, int secondsToWaitBeforeToRetry,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);

	static std::string httpPostPutFileByFormData(
		std::string url, const std::vector<std::pair<std::string, std::string>>& formData,
		const std::string& requestType, // POST or PUT
		long timeoutInSeconds, std::string pathFileName, int64_t fileSizeInBytes, const std::string& mediaContentType, std::string referenceToLog, int maxRetryNumber,
		int secondsToWaitBeforeToRetry, int64_t contentRangeStart, int64_t contentRangeEnd_Excluded,
		const std::optional<std::string> &proxyURL = std::nullopt, const std::optional<std::string> &proxyUsername = std::nullopt,
		const std::optional<std::string> &proxyPassword = std::nullopt,
		const std::optional<std::string> &httpSSLVersion = std::nullopt, bool verbose = false
	);
};
