#include <curl/curl.h>
#include <rapidjson/document.h>
#include <iostream>
#include <string>
#include <string.h>
#include <chrono>
#include <thread>


void postPoll(std::string token, std::string chatId) {
	auto curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, ("https://api.telegram.org/bot" + token + "/sendPoll").c_str());
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		auto options = "chat_id=" + chatId;
		options += "&question=\"\u0432\u043e \u0441\u043a\u043e\u043b\u044c\u043a\u043e \u0441\u0435\u0433\u043e\u0434\u043d\u044f \u0430\u043d\u0438\u043c\u0435 ?\"";
		options += "&is_anonymous=False";
		options += "&allows_multiple_answers=False";
		options += "&options=[\"20:00+\", \"20:30+\", \"21:00+\", \"21:30+\", \"22:00+\", \"22:30+\", \"23:00+\", \"23:30+\", \"00:00+\", \"-\"]";
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, options.c_str());
 
 		auto res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			fprintf(stderr, "\ncurl_easy_perform() failed: %s\n",
					curl_easy_strerror(res));
		}
 
		curl_easy_cleanup(curl);
	}
}

typedef struct {
	bool res;
	int64_t offset;
} chechResult;

size_t readMessagesCallback(char* data, size_t size, size_t nmemb,
							std::string* writerData) {
	writerData->append(data, size * nmemb);
	return size * nmemb;
}

int parseMessages(const char* data, chechResult* res, int64_t chatId) {
	rapidjson::Document document;
	document.Parse(data);
	res->res = false;
	if (!document.IsObject()) {
		return 0;
	}
	if (!document.HasMember("ok") || !document["ok"].IsBool() || !document["ok"].GetBool()) {
		return 0;
	}
	if (!document.HasMember("result") || !document["result"].IsArray()) {
		return 0;
	}
	for (const auto& upd: document["result"].GetArray()) {
		if (!upd.HasMember("update_id") || !upd["update_id"].IsNumber() || !upd["update_id"].IsInt64())
			continue;
		int64_t uid = upd["update_id"].GetInt64();
		if (uid > res->offset)
			res->offset = uid;
		if (!upd.HasMember("poll"))
			continue;
		return 0;
	}
	for (const auto& upd: document["result"].GetArray()) {
		if (!upd.HasMember("update_id") || !upd["update_id"].IsNumber() || !upd["update_id"].IsInt64())
			continue;
		int64_t uid = upd["update_id"].GetInt64();
		if (uid > res->offset)
			res->offset = uid;
		if (!upd.HasMember("message") || !upd["message"].IsObject())
			continue;
		auto msg = upd["message"].GetObject();
		if (!msg.HasMember("chat") || !msg["chat"].IsObject())
			continue;
		auto chat = msg["chat"].GetObject();
		if (!chat.HasMember("id") || !chat["id"].IsNumber() || !chat["id"].IsInt64())
			continue;
		if (!msg.HasMember("text") || !msg["text"].IsString())
			continue;
		const char* msgText = msg["text"].GetString();
		if (std::string(msgText) == "/pool@nouret_message_sender_bot" || std::string(msgText) == "/pool") { // HARDCODE :(
			res->res = true;
			return 0;
		}
	}

	return 0;
}

chechResult checkMessages(std::string token, std::string chatId, int64_t offset = 0) {
	auto curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, ("https://api.telegram.org/bot" + token + "/getUpdates").c_str());
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, readMessagesCallback);
		std::string writerData;
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writerData);
		auto options = "offset=" + std::to_string(offset) + "&limit=10";
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, options.c_str());
		auto cres = curl_easy_perform(curl);
		if(cres != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
					curl_easy_strerror(cres));
			curl_easy_cleanup(curl);
			return chechResult{false, 0L};
		} else {
			chechResult res = {false, 0L};
			int64_t chatIdL = std::stol(chatId);
			parseMessages(writerData.c_str(), &res, chatIdL);
			if (res.offset < offset) {
				curl_easy_cleanup(curl);
				return chechResult{false, res.offset};
			}
			curl_easy_cleanup(curl);
			return res;
		}

		curl_easy_cleanup(curl);
	}
	return chechResult{false, 0L};
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Arguments: id:token chat_id in format nnnnnnnnnn:ccccccccc-cccccccccccc-cccccccccccc -nnnnnnnnnn\n");
		return -1;
	}
	std::string iToken(argv[1]), chatId(argv[2]);
	int64_t offset = 0;
	auto res = checkMessages(iToken, chatId);
	int secondCounter = 0;
	while (true) {
		printf("current offset = %ld\n", offset);
		if (res.offset + 1 > offset) {
			offset = res.offset + 1;
		}
		if (res.res) {
			postPoll(iToken, chatId);
		}
		std::this_thread::sleep_for(std::chrono::seconds(5));
		secondCounter += 5;
		if (secondCounter >= 300) {
			secondCounter = 0;
			offset = 0;
		}
		res = checkMessages(iToken, chatId, offset);
	}
	return 0;
}
