#include "logger.h"
#include "http_client.h"
#include "user_handle.h"
#include "json/json_value.h"
#include "json/json_writer.h"

#include <Msi.h>
#include <sstream>
#include "zlib/zstream/izstream.h"
#include "zlib/zstream/ozstream.h"

using namespace UserOperations;

void Usage()
{
	std::wcout << "  [1]  Register" << std::endl;
	std::wcout << "  [2]  Login" << std::endl;
	std::wcout << "  [3]  Logout" << std::endl;
	std::wcout << "  [4]  Get profile" << std::endl;
	std::wcout << "  [5]  Update profile" << std::endl;
	std::wcout << "  [6]  Change password" << std::endl;
	std::wcout << "  [7]  Delete account" << std::endl;
	std::wcout << "  [8]  Upload file" << std::endl;
	std::wcout << "  [9]  Update file" << std::endl;
	std::wcout << "  [10] Remove file" << std::endl;
	std::wcout << "  [11] Rename file" << std::endl;
	std::wcout << "  [12] Upload folder" << std::endl;
	std::wcout << "  [13] Update folder" << std::endl;
	std::wcout << "  [14] Upload folder filter" << std::endl;
	std::wcout << "  [15] Update folder filter" << std::endl;
	std::wcout << "  [16] Watch folder" << std::endl;
	std::wcout << "  [E]xit program" << std::endl;
	std::wcout << "=> Action number: ";
}

void cmd_json_setup(const std::wstring& config_path, std::unique_ptr<UserHandle>& handler, std::unique_ptr<HttpClient>& net);
void cmd_user_setup(std::unique_ptr<UserHandle>& handler, std::unique_ptr<HttpClient>& net);
void cmd_user_action(std::unique_ptr<UserHandle>& handler, std::unique_ptr<HttpClient>& net);

void cmd_user_register(std::unique_ptr<UserHandle>& handler);
void cmd_user_login(std::unique_ptr<UserHandle>& handler);
void cmd_user_logout(std::unique_ptr<UserHandle>& handler);
void cmd_user_get_profile(std::unique_ptr<UserHandle>& handler);
void cmd_user_update_profile(std::unique_ptr<UserHandle>& handler);
void cmd_user_change_password(std::unique_ptr<UserHandle>& handler);
void cmd_user_delete_account(std::unique_ptr<UserHandle>& handler);
void cmd_user_upload_file(std::unique_ptr<UserHandle>& handler);
void cmd_user_update_file(std::unique_ptr<UserHandle>& handler);
void cmd_user_upload_folder(std::unique_ptr<UserHandle>& handler);
void cmd_user_update_folder(std::unique_ptr<UserHandle>& handler);
void cmd_user_update_folder_filter(std::unique_ptr<UserHandle>& handler);
void cmd_user_upload_folder_filter(std::unique_ptr<UserHandle>& handler);
void cmd_user_remove_file(std::unique_ptr<UserHandle>& handler);
void cmd_user_rename_file(std::unique_ptr<UserHandle>& handler);
void cmd_user_watch_folder(std::unique_ptr<UserHandle>& handler);

void TEST()
{
	FolderInfo folder;
	std::wstring folder_path = L"E:\\DEV\\SE33\\Resource\\Cloud_Storage\\folder_sync";
	FolderHandle::GetFolderInfo(folder_path, folder);

	auto list = folder.GetFilesRecursive();
	std::string json = JsonUtility::CreateJsonFolderTree(folder);

	FolderHandle::GetFolderFilter(folder_path, L".bin", folder);
	folder.CountFile();

	for (int i = 0; i < folder.CountChildren(); i++)
	{
		FolderInfo children = folder.GetChildren(i);
		children.SetFolderName(L"abc");
		for (int j = 0; j < children.CountFile(); j++)
		{
			FileInfo file = children.GetFile(j);
			file.SetFileName(L"xyz");
		}
	}
}

void TEST_1()
{
	UserHandle handler;
	std::wstring folder_path = L"E:\\DEV\\SE33\\Resource\\Cloud_Storage\\folder_sync";

	handler.MonitorFolder(folder_path);

	//handler.WatchFolderSync(folder_path, L"*");
}

void TEST_2() 
{
	std::string input = "Hello world \n";
	std::ostringstream buffer;
	{
		zstream::ozstream zipper(buffer);
		zipper << input;
	}
	std::string buffer_compress = buffer.str();

	std::istringstream ibuffer(buffer_compress);
	zstream::izstream izstream(ibuffer);
	izstream.unsetf(std::ios_base::skipws);

	std::string output = std::string((std::istream_iterator<char>(izstream)), std::istream_iterator<char>());

	if (output != input)
	{
		std::cout << "Compare failed" << std::endl;
		return;
	}
	else
	{
		std::cout << "Compare succeeded" << std::endl;
		return;
	}
}

void TEST_3() 
{
	FolderInfo folder;
	std::wstring folder_path = L"E:\\DEV\\SE33\\Resource\\Cloud_Storage\\folder_sync";
	FolderHandle::GetFolderInfo(folder_path, folder);
	std::string tree = JsonUtility::CreateJsonFolderTree(folder);
	std::cout << tree << std::endl;
}

int wmain(int argc, wchar_t* argv[])
{
	ENABLE_LOG(TRUE);
	SET_LOG_OUT(SHOW_MESSAGE);
	SET_LOG_LEVEL(SUCCS_LEVEL);
	
	//TEST();
	TEST_1();
	//TEST_2();
	//TEST_3();

	//std::unique_ptr<UserHandle> handler = std::make_unique<UserHandle>();
	//std::unique_ptr<HttpClient> net = std::make_unique<HttpClient>();
	//net->OptionKeepConnect(TRUE);
	//net->OptionConnectTimeOut(300000);
	//if (argc == 2)
	//{
	//	cmd_json_setup(argv[1], handler, net);
	//}
	//else
	//{
	//	cmd_user_setup(handler, net);
	//}
	//cmd_user_action(handler, net);

	return 0;
}

void cmd_json_setup(const std::wstring& config_path, std::unique_ptr<UserHandle>& handler, std::unique_ptr<HttpClient>& net) 
{
	int port = 0;				// -port		8080
	std::wstring host;			// -host		"localhost"
	std::wstring protocol;		// -protocol	HTTP
	std::wstring cert_name;		// -cert_name	"localhost"
	std::wstring cert_store;	// -cert_store	"Root"
	std::wstring cert_path;		// -cert_path	if not import from store -> import from file: ".../folder/client.pfx"
	std::wstring cert_key;		// -cert_key	"qwerty"	
	std::wstring file_cache;	// -cache		".../folder/file.txt"

	BYTE* buffer = NULL;
	DWORD buffer_size = 0;
	if (FileHandle::ReadFileData(config_path, buffer, buffer_size))
	{
		std::string message = std::string((char*)buffer, buffer_size);
		JsonValue* jr = JsonParser::Parse(message.c_str());
		if (jr->IsObject() && jr->CountChildren() > 0)
		{
			port = (int)jr->Child(L"port_number")->AsNumber();
			host = jr->Child(L"host_name")->AsString();
			protocol = jr->Child(L"protocol")->AsString();
			cert_name = jr->Child(L"cert_name")->AsString();
			cert_store = jr->Child(L"cert_store")->AsString();
			cert_path = jr->Child(L"cert_path")->AsString();
			cert_key = jr->Child(L"cert_key")->AsString();
			file_cache = jr->Child(L"file_cache")->AsString();
		}
		if (jr)
		{
			delete jr;
		}
		if (buffer)
		{
			delete[] buffer;
		}
	}
	if (Helper::StringHelper::convertToLowerCase(protocol) == L"http")
	{
		net->Connect(host, port, FALSE);
	}
	else if (Helper::StringHelper::convertToLowerCase(protocol) == L"https")
	{
		PCCERT_CONTEXT cert_context = NULL;
		std::wcout << L"Importing certificate from file..." << std::endl;
		cert_context = CertificateManager::Instance()->ImportLoadCertFromFile(cert_path, cert_key, cert_name);
		if (cert_context == NULL)
		{
			std::wcout << L"Failed to import certificate from file!" << std::endl;
			return;
		}
		net->Connect(host, port, TRUE, cert_context);
	}
	// Setup connect network
	handler->SetupNetwork(net.get());
	// Setup file cache
	FileCache* cache = new FileCache(file_cache);
	handler->SetupFileCache(cache);
}
void cmd_user_setup(std::unique_ptr<UserHandle>& handler, std::unique_ptr<HttpClient>& net)
{
	int port = 0;
	std::wstring host;
	std::wstring protocol;
	std::wstring file_cache;
	do
	{
		std::wcout << L"Enter host (eg: localhost): ";
		std::getline(std::wcin, host);
		if (host.empty())
		{
			std::wcout << L"Invalid input! Please enter a valid host name." << std::endl;
			continue;
		}

		std::wstring input;
		std::wcout << L"Enter port (eg: 8080, 8443): ";
		std::getline(std::wcin, input);
		if (!Helper::StringHelper::isStringNumeric(input))
		{
			std::wcout << L"Invalid input! Please enter a valid port number." << std::endl;
			continue;
		}
		port = _wtoi(input.c_str());

		std::wcout << L"Choose protocol (HTTP, HTTPS): ";
		std::getline(std::wcin, protocol);
		if (protocol.empty())
		{
			std::wcout << L"Invalid input! Please enter a valid protocol name." << std::endl;
			continue;
		}

		std::wcout << L"Enter path file cache: ";
		std::getline(std::wcin, file_cache);
		if (file_cache.empty())
		{
			std::wcout << L"Invalid input! Please enter a valid path." << std::endl;
			continue;
		}

		if (Helper::StringHelper::convertToLowerCase(protocol) == L"http")
		{
			net->Connect(host, port, FALSE);
		}
		else if (Helper::StringHelper::convertToLowerCase(protocol) == L"https")
		{
			// Input certificate information
			PCCERT_CONTEXT cert_context = NULL;
			std::wstring cert_name, cert_store, cert_path, cert_key, import_choice;
			do
			{
				std::wcout << L"Do you want to (1) get cert from store or (2) import cert from file? (Enter 1 or 2): ";
				std::getline(std::wcin, import_choice);

				if (import_choice == L"1")
				{
					std::wcout << L"Enter certificate name (e.g: localhost): ";
					std::getline(std::wcin, cert_name);
					std::wcout << L"Enter certificate store (e.g: Root): ";
					std::getline(std::wcin, cert_store);

					cert_context = CertificateManager::Instance()->GetCertificateFromStore(cert_name, cert_store, CERT_SYSTEM_STORE_LOCAL_MACHINE);
					if (cert_context == NULL)
					{
						std::wcout << L"Can't load cert from store! Please check the certificate name and store." << std::endl;
					}
				}
				else if (import_choice == L"2")
				{
					std::wcout << L"Enter certificate name: ";
					std::getline(std::wcin, cert_name);
					std::wcout << L"Enter certificate path: ";
					std::getline(std::wcin, cert_path);
					std::wcout << L"Enter certificate key: ";
					std::getline(std::wcin, cert_key);

					std::wcout << L"Importing certificate from file..." << std::endl;
					cert_context = CertificateManager::Instance()->ImportLoadCertFromFile(cert_path, cert_key, cert_name);
					if (cert_context == NULL)
					{
						std::wcout << L"Failed to import certificate from file!" << std::endl;
					}
				}
				else
				{
					std::wstring action;
					std::wcout << L"Invalid choice! Enter [e]xit" << std::endl;
					std::getline(std::wcin, action);
					if (wcscmp(Helper::StringHelper::convertToLowerCase(action).c_str(), L"e") == 0)
					{
						return;
					}
					continue;
				}
			} while (true);

			// Connect using the certificate if it was successfully retrieved or imported
			if (cert_context != NULL)
			{
				net->Connect(host, port, TRUE, cert_context);
			}
			else
			{
				std::wstring action;
				std::wcout << L"Can't load cert from store! Enter [e]xit" << std::endl;
				std::getline(std::wcin, action);
				if (wcscmp(Helper::StringHelper::convertToLowerCase(action).c_str(), L"e") == 0)
				{
					return;
				}
				continue;
			}
		}
		else
		{
			std::wstring action;
			std::wcout << L"Invalid choice! Enter [e]xit" << std::endl;
			std::getline(std::wcin, action);
			if (wcscmp(Helper::StringHelper::convertToLowerCase(action).c_str(), L"e") == 0)
			{
				return;
			}
			continue;
		}
		break;
	} while (true);

	// Setup connect network
	handler->SetupNetwork(net.get());
	// Setup file cache
	FileCache* cache = new FileCache(file_cache);
	handler->SetupFileCache(cache);
}
void cmd_user_action(std::unique_ptr<UserHandle>& handler, std::unique_ptr<HttpClient>& net)
{
	int number = 0;
	while (true)
	{
		std::wcout << L"\n===================[ Action ]===================" << std::endl;
		Usage();

		std::wstring input;
		std::getline(std::wcin, input);
		if (wcscmp(Helper::StringHelper::convertToLowerCase(input).c_str(), L"e") == 0)
		{
			if (net)
			{
				net->Disconnect();
			}
			return;
		}
		number = _wtoi(input.c_str());
		switch (number)
		{
			case 1:
				std::wcout << L"\n=================[ Register ]===================" << std::endl;
				cmd_user_register(handler);
				break;
			case 2:
				std::wcout << L"\n===================[ Login ]====================" << std::endl;
				cmd_user_login(handler);
				break;
			case 3:
				std::wcout << L"\n===================[ Logout ]===================" << std::endl;
				cmd_user_logout(handler);
				break;
			case 4:
				std::wcout << L"\n=================[ Get profile ]================" << std::endl;
				cmd_user_get_profile(handler);
				break;
			case 5:
				std::wcout << L"\n================[ Update profile ]==============" << std::endl;
				cmd_user_update_profile(handler);
				break;
			case 6:
				std::wcout << L"\n================[ Change password ]=============" << std::endl;
				cmd_user_change_password(handler);
				break;
			case 7:
				std::wcout << L"\n=================[ Delete account ]=============" << std::endl;
				cmd_user_delete_account(handler);
				break;
			case 8:
				std::wcout << L"\n=================[ Upload file ]================" << std::endl;
				cmd_user_upload_file(handler);
				break;
			case 9:
				std::wcout << L"\n=================[ Update file ]================" << std::endl;
				cmd_user_update_file(handler);
				break;
			case 10:
				std::wcout << L"\n=================[ Remove file ]================" << std::endl;
				cmd_user_remove_file(handler);
				break;
			case 11:
				std::wcout << L"\n=================[ Rename file ]================" << std::endl;
				cmd_user_rename_file(handler);
				break;
			case 12:
				std::wcout << L"\n=================[ Upload folder ]==============" << std::endl;
				cmd_user_upload_folder(handler);
				break;
			case 13:
				std::wcout << L"\n============[ Update folder filter ]============" << std::endl;
				cmd_user_update_folder_filter(handler);
				break;
			case 14:
				std::wcout << L"\n============[ Upload folder filter ]============" << std::endl;
				cmd_user_upload_folder_filter(handler);
				break;
			case 15:
				std::wcout << L"\n=================[ Update folder ]==============" << std::endl;
				cmd_user_update_folder(handler);
				break;
			case 16:
				std::wcout << L"\n=============[ Watch folder sync ]==============" << std::endl;
				cmd_user_watch_folder(handler);
				break;
			default:
				std::wcout << L"=> Invalid input!" << std::endl;
				break;
		}
	}
}

void cmd_user_register(std::unique_ptr<UserHandle>& handler)
{
	std::unique_ptr<UserInfo> user_info = std::make_unique<UserInfo>();

	// Input user information
	std::wcout << L"Enter first name: ";
	std::getline(std::wcin, user_info->first_name);
	std::wcout << L"Enter last name: ";
	std::getline(std::wcin, user_info->last_name);
	std::wcout << L"Enter user name: ";
	std::getline(std::wcin, user_info->user_name);
	std::wcout << L"Enter password: ";
	std::getline(std::wcin, user_info->password);
	std::wcout << L"Enter email: ";
	std::getline(std::wcin, user_info->email);
	std::wcout << L"Enter birthdate (YYYY-MM-DD): ";
	std::getline(std::wcin, user_info->birthday);

	if (!handler->RegisterAccount(*user_info.get()))
	{
		return;
	}
}
void cmd_user_login(std::unique_ptr<UserHandle>& handler)
{
	std::wstring user_name, password;
	std::wcout << L"Enter user name: "; std::getline(std::wcin, user_name);
	std::wcout << L"Enter password: "; std::getline(std::wcin, password);

	// Input user information
	if (!handler->LoginAccount(user_name, password))
	{
		return;
	}
}
void cmd_user_logout(std::unique_ptr<UserHandle>& handler)
{
	if (!handler->LogoutAccount())
	{
		return;
	}
}
void cmd_user_get_profile(std::unique_ptr<UserHandle>& handler)
{
	UserInfo profile;
	if (!handler->GetUserProfile(profile))
	{
		return;
	}
}
void cmd_user_update_profile(std::unique_ptr<UserHandle>& handler)
{
	UserInfo profile;

	// Input user information with an option to skip
	std::wcout << L"Enter first name (or press Enter to skip): ";
	std::getline(std::wcin, profile.first_name);

	std::wcout << L"Enter last name (or press Enter to skip): ";
	std::getline(std::wcin, profile.last_name);

	std::wcout << L"Enter email (or press Enter to skip): ";
	std::getline(std::wcin, profile.email);

	std::wcout << L"Enter birthdate (YYYY-MM-DD) (or press Enter to skip): ";
	std::getline(std::wcin, profile.birthday);

	// Update user profile only if at least one field is filled
	if (profile.first_name.empty() && profile.last_name.empty() && profile.user_name.empty() &&
		profile.password.empty() && profile.email.empty() && profile.birthday.empty())
	{
		std::wcout << L"No information provided. Profile update skipped." << std::endl;
		return;
	}

	if (!handler->UpdateUserProfile(profile))
	{
		return;
	}
}
void cmd_user_change_password(std::unique_ptr<UserHandle>& handler)
{
	std::wstring old_password;
	std::wstring new_password;
	std::wcout << L"Enter old password: "; std::getline(std::wcin, old_password);
	std::wcout << L"Enter new password: "; std::getline(std::wcin, new_password);
	if (!handler->ChangePassword(old_password, new_password))
	{
		return;
	}
}
void cmd_user_delete_account(std::unique_ptr<UserHandle>& handler)
{
	if (!handler->DeleteAccount())
	{
		return;
	}
}
void cmd_user_upload_file(std::unique_ptr<UserHandle>& handler)
{
	FileInfo file;
	std::wstring file_path;
	std::wcout << L"Enter file path: "; std::getline(std::wcin, file_path);

	FileHandle::GetFileInfo(file_path, file);
	if (!handler->UploadFile(file))
	{
		return;
	}
}
void cmd_user_update_file(std::unique_ptr<UserHandle>& handler)
{
	FileInfo file;
	std::wstring file_path;
	std::wcout << L"Enter file path: "; std::getline(std::wcin, file_path);

	FileHandle::GetFileInfo(file_path, file);
	if (!handler->UpdateFile(file))
	{
		return;
	}	
}
void cmd_user_upload_folder(std::unique_ptr<UserHandle>& handler)
{
	FolderInfo folder;
	std::wstring folder_path;
	std::wcout << L"Enter folder path: "; std::getline(std::wcin, folder_path);

	FolderHandle::GetFolderInfo(folder_path, folder);
	if (!handler->UploadFolder(folder))
	{
		return;
	}
}
void cmd_user_update_folder(std::unique_ptr<UserHandle>& handler)
{
	FolderInfo folder;
	std::wstring folder_path;
	std::wcout << L"Enter folder path: "; std::getline(std::wcin, folder_path);

	FolderHandle::GetFolderInfo(folder_path, folder);
	if (!handler->UpdateFolder(folder))
	{
		return;
	}
}
void cmd_user_upload_folder_filter(std::unique_ptr<UserHandle>& handler)
{
	std::wstring folder_path;
	std::wcout << L"Enter folder path: "; std::getline(std::wcin, folder_path);
	std::wstring filter_string;
	std::wcout << L"Enter filter search (eg: *.txt): "; std::getline(std::wcin, filter_string);

	if (!handler->UploadFolderWithFilter(folder_path, filter_string))
	{
		return;
	}
}
void cmd_user_update_folder_filter(std::unique_ptr<UserHandle>& handler)
{
	std::wstring folder_path;
	std::wcout << L"Enter folder path: "; std::getline(std::wcin, folder_path);
	std::wstring filter_string;
	std::wcout << L"Enter filter search (eg: *.txt): "; std::getline(std::wcin, filter_string);

	if (!handler->UpdateFolderWithFilter(folder_path, filter_string))
	{
		return;
	}
}
void cmd_user_remove_file(std::unique_ptr<UserHandle>& handler)
{
	std::wstring file_path;
	std::wcout << L"Enter file path: "; std::getline(std::wcin, file_path);
	if (!handler->RemoveFile(file_path))
	{
		return;
	}
}
void cmd_user_rename_file(std::unique_ptr<UserHandle>& handler)
{
	std::wstring file_path;
	std::wcout << L"Enter file path: "; std::getline(std::wcin, file_path);
	std::wstring new_file_name;
	std::wcout << L"Enter new file name: "; std::getline(std::wcin, new_file_name);
	if (!handler->RenameFile(file_path, new_file_name))
	{
		return;
	}
}
void cmd_user_watch_folder(std::unique_ptr<UserHandle>& handler)
{
	std::wstring sleep_str;
	std::wstring folder_path;
	std::wcout << L"Enter folder path: "; std::getline(std::wcin, folder_path);
	if (!handler->WatchFolderSync(folder_path))
	{
		return;
	}
}
