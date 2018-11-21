// Tryouts.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#define DEBUG

#include <iostream>

#include "User.h"

#include "GetDocumentsCommand.h"
#include "PutDocumentCommand.h"
#include "RequestExecutor.h"
#include "GetDatabaseRecordCommand.h"
#include "CreateDatabaseCommand.h"

#include "GetDatabaseNamesCommand.h"
#include "DeleteDatabaseCommand.h"
#include "DeleteDocumentCommand.h"

namespace
{
	//using fiddler + verbose
	void set_for_fiddler(CURL* curl)
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1:8888");
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	}



}

int main()
{
	using namespace ravendb::client;

	//User user{ "Alexander","Timoshenko" };
	//nlohmann::json j = user;
	//std::string id{ "newID123" };
	//auto _executor = ravendb::client::http::RequestExecutor::create({ "http://127.0.0.1:8080" }, "TEST__DocumentCommandsTests", {},
	//	{ debug_curl_init, nullptr });
	//{
	//	documents::commands::PutDocumentCommand cmd(id, {}, j);
	//	_executor->execute(cmd);
	//}
	//{
	//	documents::commands::DeleteDocumentCommand cmd(id,"dssdf");
	//	_executor->execute(cmd);
	//}

	auto exec = http::RequestExecutor::create({ "http://127.0.0.1:8080" }, {}, {}, set_for_fiddler);
	serverwide::DatabaseRecord rec;
	rec.database_name = "test";
	serverwide::operations::CreateDatabaseCommand cmd(rec,1);

	auto res1 = exec->execute(cmd);

	for (auto& it : res1.topology.promotables_status)
	{
		std::cout << static_cast<uint8_t>(it.second)<<" ";
	}

	//auto test_suite_executor = http::RequestExecutor::create({ "http://127.0.0.1:8080" }, "Test", {}, set_for_fiddler);
	//std::string id1(500,'a'), id2 = id1+"aaa", id3 = id2+"AAA";

	//documents::commands::GetDocumentsCommand cmd({id1,id2,id3}, {}, true);
	//test_suite_executor->execute(cmd);

	//auto test_suite_executor = http::RequestExecutor::create({ "http://127.0.0.1:8080" }, {}, {}, set_for_fiddler);
	serverwide::operations::GetDatabaseRecordCommand cmd2("test");

	auto res2 = exec->execute(cmd2);

	getchar();

}

