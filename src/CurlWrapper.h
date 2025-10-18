/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   CURL.h
 * Author: giuliano
 *
 * Created on March 29, 2018, 6:27 AM
 */

#pragma once

#include <cstdint>
#include <stdexcept>
#include <string_view>
#ifndef SPDLOG_ACTIVE_LEVEL
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"
#include <curl/curl.h>
#include <deque>
#include <fstream>
#include <vector>

using namespace std;

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;
using namespace nlohmann::literals;

struct CurlException : public runtime_error
{
	explicit CurlException(const string &message) : runtime_error(message) {};
	~CurlException() noexcept override = default;

	[[nodiscard]] virtual string_view type() const noexcept { return "CurlException"; }
};

struct ServerNotReachable : public CurlException
{
	explicit ServerNotReachable(const string &message) : CurlException(message) {};
	~ServerNotReachable() noexcept override = default;

	[[nodiscard]] virtual string_view type() const noexcept override { return "ServerNotReachable"; }
};

struct HTTPError : public CurlException
{
	int16_t httpErrorCode;
	HTTPError(int16_t httpErrorCode, const string &message) : CurlException(message), httpErrorCode(httpErrorCode) {};
	~HTTPError() noexcept override = default;

	[[nodiscard]] string_view type() const noexcept override { return "HTTPError"; }
};

class CurlWrapper
{

  public:
	struct CurlDownloadData
	{
		string referenceToLog;
		int currentChunkNumber;
		string destBinaryPathName;
		ofstream mediaSourceFileStream;
		size_t currentTotalSize;
		size_t maxChunkFileSize;
	};

	struct CurlUploadData
	{
		ifstream mediaSourceFileStream;

		int64_t payloadBytesSent;
		int64_t upToByte_Excluded;
	};

	struct CurlUploadFormData
	{
		ifstream mediaSourceFileStream;

		int64_t payloadBytesSent;
		int64_t upToByte_Excluded;

		bool formDataSent;
		string formData;

		bool endOfFormDataSent;
		string endOfFormData;
	};

	struct CurlUploadEmailData
	{
		deque<string> emailLines;
	};

	static void globalInitialize();

	static void globalTerminate();

	static string escape(const string &url);
	static string unescape(const string &url);

	static string basicAuthorization(const string &user, const string &password);
	static string bearerAuthorization(const string &bearerToken);

	static void httpGetBinary(
		string url, long timeoutInSeconds, string authorization, const vector<string>& otherHeaders, string referenceToLog, int maxRetryNumber,
		int secondsToWaitBeforeToRetry, vector<uint8_t> &binary
	);

	static string httpGet(
		const string &url, long timeoutInSeconds, const string &authorization = "", const vector<string> &otherHeaders = vector<string>(),
		const string &referenceToLog = "",
		int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15
	);

	static json httpGetJson(
		const string& url, long timeoutInSeconds, const string& authorization = "", const vector<string>& otherHeaders = vector<string>(), const string& referenceToLog = "",
		int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15, bool outputCompressed = false
	);

	static string httpDelete(
		string url, long timeoutInSeconds, string authorization, const vector<string>& otherHeaders = vector<string>(), string referenceToLog = "",
		int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15
	);

	static string httpPostString(
		const string& url, long timeoutInSeconds, const string& authorization, const string& body,
		const string& contentType, // i.e.: application/json
		const vector<string>& otherHeaders, const string& referenceToLog, int maxRetryNumber, int secondsToWaitBeforeToRetry, bool outputCompressed
	);

	static string httpPutString(
		const string& url, long timeoutInSeconds, const string& authorization, const string &body,
		const string& contentType, // i.e.: application/json
		const vector<string>& otherHeaders, const string& referenceToLog, int maxRetryNumber, int secondsToWaitBeforeToRetry, bool outputCompressed
	);

	static pair<string, string> httpPostString(
		const string &url, long timeoutInSeconds, const string &authorization, const string& body, const string &contentType = "application/json",
		const vector<string>& otherHeaders = vector<string>(), const string &referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15
	);

	static pair<string, string> httpPutString(
		const string &url, long timeoutInSeconds, const string &authorization, const string& body, const string &contentType = "application/json",
		const vector<string>& otherHeaders = vector<string>(), const string &referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15
	);

	static json httpPostStringAndGetJson(
		const string& url, long timeoutInSeconds, const string &authorization, const string &body, const string& contentType = "application/json",
		const vector<string>& otherHeaders = vector<string>(), const string& referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15,
		bool outputCompressed = false
	);

	static json httpPutStringAndGetJson(
		const string& url, long timeoutInSeconds, const string &authorization, const string &body, const string& contentType = "application/json",
		const vector<string>& otherHeaders = vector<string>(), const string& referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15,
		bool outputCompressed = false
	);

	static string httpPostFile(
		const string& url, long timeoutInSeconds, const string& authorization, const string& pathFileName,
		uintmax_t fileSizeInBytes, const string& contentType = "",
		const string& referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15, int64_t contentRangeStart = -1,
		int64_t contentRangeEnd_Excluded = -1
	);

	static string httpPutFile(
		string url, long timeoutInSeconds, const string &authorization, const string& pathFileName, int64_t fileSizeInBytes, const string& contentType = "",
		const string& referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15, int64_t contentRangeStart = -1,
		int64_t contentRangeEnd_Excluded = -1
	);

	static json httpPostFileAndGetJson(
		string url, long timeoutInSeconds, const string& authorization, const string &pathFileName, int64_t fileSizeInBytes, const string& referenceToLog = "",
		int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15, int64_t contentRangeStart = -1, int64_t contentRangeEnd_Excluded = -1
	);

	static json httpPutFileAndGetJson(
		const string &url, long timeoutInSeconds, const string& authorization, const string& pathFileName, int64_t fileSizeInBytes, const string& referenceToLog = "",
		int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15, int64_t contentRangeStart = -1, int64_t contentRangeEnd_Excluded = -1
	);

	static string httpPostFileSplittingInChunks(
		const string& url, long timeoutInSeconds, const string& authorization, const string &pathFileName, const function<bool(int, int)>& chunkCompleted,
		const string& referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15
	);

	static string httpPostFormData(
		const string &url, const vector<pair<string, string>> &formData, long timeoutInSeconds, const string &referenceToLog = "", int maxRetryNumber = 0,
		int secondsToWaitBeforeToRetry = 15
	);

	static string httpPutFormData(
		const string &url, const vector<pair<string, string>> &formData, long timeoutInSeconds, const string &referenceToLog = "", int maxRetryNumber = 0,
		int secondsToWaitBeforeToRetry = 15
	);

	static json httpPostFormDataAndGetJson(
		const string &url, const vector<pair<string, string>>& formData, long timeoutInSeconds, const string& referenceToLog = "", int maxRetryNumber = 0,
		int secondsToWaitBeforeToRetry = 15
	);

	static json httpPutFormDataAndGetJson(
		const string &url, const vector<pair<string, string>>& formData, long timeoutInSeconds, const string &referenceToLog = "", int maxRetryNumber = 0,
		int secondsToWaitBeforeToRetry = 15
	);

	static string httpPostFileByFormData(
		const string &url, const vector<pair<string, string>> &formData, long timeoutInSeconds, const string &pathFileName, int64_t fileSizeInBytes,
		const string &mediaContentType, const string &referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15,
		int64_t contentRangeStart = -1, int64_t contentRangeEnd_Excluded = -1
	);

	static string httpPutFileByFormData(
		const string &url, const vector<pair<string, string>> &formData, long timeoutInSeconds, const string &pathFileName, int64_t fileSizeInBytes,
		const string &mediaContentType, const string &referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15,
		int64_t contentRangeStart = -1, int64_t contentRangeEnd_Excluded = -1
	);

	static json httpPostFileByFormDataAndGetJson(
		const string& url, const vector<pair<string, string>> &formData, long timeoutInSeconds, const string& pathFileName, int64_t fileSizeInBytes,
		const string& mediaContentType, const string& referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15,
		int64_t contentRangeStart = -1, int64_t contentRangeEnd_Excluded = -1
	);

	static json httpPutFileByFormDataAndGetJson(
		const string& url, const vector<pair<string, string>>& formData, long timeoutInSeconds, const string &pathFileName, int64_t fileSizeInBytes,
		const string& mediaContentType, const string& referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15,
		int64_t contentRangeStart = -1, int64_t contentRangeEnd_Excluded = -1
	);

	static void downloadFile(
		string url, string destBinaryPathName, int (*progressCallback)(void *, curl_off_t, curl_off_t, curl_off_t, curl_off_t), void *progressData,
		long downloadChunkSizeInMegaBytes = 500, string referenceToLog = "", long timeoutInSeconds = 120, int maxRetryNumber = 0,
		bool resumeActive = false, int secondsToWaitBeforeToRetry = 15
	);

	static void ftpFile(
		string filePathName, const string& fileName, int64_t sizeInBytes, string ftpServer, int ftpPort, string ftpUserName, string ftpPassword,
		string ftpRemoteDirectory, const string& ftpRemoteFileName, int (*progressCallback)(void *, curl_off_t, curl_off_t, curl_off_t, curl_off_t),
		void *progressData, string referenceToLog = "", int maxRetryNumber = 0, int secondsToWaitBeforeToRetry = 15
	);

	static void sendEmail(
		string emailServerURL, const string& useName, const string& password, string from, string tosCommaSeparated, string ccsCommaSeparated, string subject,
		vector<string> &emailBody, string contentType
	);

  private:
	static pair<string, string> httpPostPutString(
		string url,
		const string& requestType, // POST or PUT
		long timeoutInSeconds, string authorization, const string& body,
		string contentType, // i.e.: application/json
		const vector<string>& otherHeaders, string referenceToLog, int maxRetryNumber, int secondsToWaitBeforeToRetry
	);

	static void httpPostPutBinary(
		const string& url,
		const string& requestType, // POST or PUT
		long timeoutInSeconds, const string& authorization, const string& body,
		const string& contentType, // i.e.: application/json
		const vector<string>& otherHeaders, const string& referenceToLog, int maxRetryNumber,
		int secondsToWaitBeforeToRetry, vector<uint8_t> &binary
	);

	static string httpPostPutFile(
		const string& url,
		const string& requestType, // POST or PUT
		long timeoutInSeconds, const string& authorization, const string& pathFileName, int64_t fileSizeInBytes,
		const string& contentType, const string& referenceToLog,
		int maxRetryNumber, int secondsToWaitBeforeToRetry, int64_t contentRangeStart, int64_t contentRangeEnd_Excluded
	);

	static string httpPostPutFormData(
		string url, const vector<pair<string, string>>& formData,
		const string& requestType, // POST or PUT
		long timeoutInSeconds, string referenceToLog, int maxRetryNumber, int secondsToWaitBeforeToRetry
	);

	static string httpPostPutFileByFormData(
		string url, const vector<pair<string, string>>& formData,
		const string& requestType, // POST or PUT
		long timeoutInSeconds, string pathFileName, int64_t fileSizeInBytes, const string& mediaContentType, string referenceToLog, int maxRetryNumber,
		int secondsToWaitBeforeToRetry, int64_t contentRangeStart, int64_t contentRangeEnd_Excluded
	);
};
