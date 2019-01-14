#pragma once
// Copyright (c) 2019 Say-Y(Hani Kim(Kim Han Byeol))
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files(the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following

// conditions :
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#include <Windows.h>
#include <mutex>
#include <iostream>
#include <cassert>
#include <map>

namespace ACIO
{
	enum class EKeyType
	{
		Char,
		Word,
		Int,
		Float,
		Double,
		Int64,
		End
	};
	class AsyncConsoleIO * g_AsyncConsoleIO = nullptr;

	class AsyncConsoleIO
	{
	private:
		struct ACIOData
		{
			void** ppData = nullptr;
			void** ppUserInputData = nullptr;
			EKeyType eKeyType = EKeyType::End;
			bool bForced = false;
		};

	private:
		std::mutex	mtx_lock = {};
		HANDLE		m_hThread = nullptr;
		DWORD		m_dwThreadId = 0;
		std::map<std::string, ACIOData> m_mapOrder;

		bool m_bLoop = true;

	private:
		static DWORD WINAPI ThreadFunc(LPVOID lpParam)
		{
			AsyncConsoleIO * pInstance = AsyncConsoleIO::GetInst();
			FILE * pConIO = nullptr;
			FILE * pConOut = nullptr;
			FILE * pConErr = nullptr;

			if (AllocConsole())
			{
				freopen_s(&pConIO, "CONIN$", "rb", stdin);
				freopen_s(&pConOut, "CONOUT$", "wb", stdout);
				freopen_s(&pConErr, "CONOUT$", "wb", stderr);
			}
			pInstance->Loop();

			if (pConIO)
			{
				fclose(pConIO);
				pConIO = nullptr;
			}
			if (pConOut)
			{
				fclose(pConOut);
				pConIO = nullptr;
			}
			if (pConErr)
			{
				fclose(pConErr);
				pConIO = nullptr;
			}

			FreeConsole();

			return 0;
		}

	public:
		void set_lock()
		{
			mtx_lock.lock();
		}

		void set_unlock()
		{
			mtx_lock.unlock();
		}

		/// <summary> Replaces an existing pointer if a duplicate key exists. </summary>
		/// <param name="key"> Key to enter in console. </param>
		/// <param name="ppValue"> Address of the variable. </param>
		/// <param name="iTypeSize"> Must be one of 1,2,3,4,8 </param>
		void bind_data(std::string key, void ** ppValue, EKeyType eKeyType)
		{
			if (!m_hThread) assert(false);
			if (key.size() == 0) assert(false);

			set_lock();

			auto iter = m_mapOrder.find(key);
			if (iter == m_mapOrder.end())
			{
				m_mapOrder.insert(make_pair(key, ACIOData{ ppValue, nullptr, eKeyType, false }));
			}
			else
			{
				if ((*iter).second.ppData != ppValue)
					assert(false && "bind_data: It is different from the address previously registered.");

				if ((*iter).second.bForced)
					assert(false && "bind_data: Do not use with bind_data_forced.");

				if ((*iter).second.eKeyType != eKeyType)
					assert(false && "bind_data: Type is different.");

				(*iter).second.ppData = ppValue;
				(*iter).second.eKeyType = eKeyType;
			}

			set_unlock();
		}

		/// <summary> It is overwritten with the value entered at the console at the time of the call.</summary>
		/// <param name="key"> Key to enter in console. </param>
		/// <param name="ppValue"> Address of the variable. </param>
		/// <param name="iTypeSize"> Must be one of 1,2,3,4,8 </param>
		void bind_data_forced(std::string key, void ** ppValue, EKeyType eKeyType)
		{
			if (!m_hThread) assert(false);
			if (key.size() == 0) assert(false);

			set_lock();

			auto iter = m_mapOrder.find(key);
			if (iter == m_mapOrder.end())
			{
				m_mapOrder.insert(make_pair(key, ACIOData{ ppValue, nullptr, eKeyType, true }));
			}
			else
			{
				if ((*iter).second.ppData != ppValue)
					assert(false && "bind_data_forced: It is different from the address previously registered.");

				if (!(*iter).second.bForced)
					assert(false && "bind_data_forced: Do not use with bind_data.");

				if((*iter).second.eKeyType != eKeyType)
					assert(false && "bind_data_forced: Type is different.");

				(*iter).second.ppData = ppValue;

				if ((*iter).second.ppUserInputData)
				{
					*((*iter).second.ppData) = *((*iter).second.ppUserInputData);
				}
			}

			set_unlock();
		}

		static AsyncConsoleIO * GetInst()
		{
			if(!g_AsyncConsoleIO) g_AsyncConsoleIO = new AsyncConsoleIO;

			return g_AsyncConsoleIO;
		}

		void DestroyInst()
		{
			delete g_AsyncConsoleIO;
		}

	private:
		template <typename T>
		void user_input(ACIOData & tACIOData)
		{
			using namespace std;
			T inputData = 0;
			cin >> inputData;

			mtx_lock.lock();

			T* pData = (T*)tACIOData.ppData;
			*pData = inputData;

			// save user input
			if (tACIOData.bForced)
			{
				if (!tACIOData.ppUserInputData)
				{
					T* tUserInputData = new T;
					tACIOData.ppUserInputData = (void**)tUserInputData;
				}

				T* pUserInputData = (T*)tACIOData.ppUserInputData;
				*pUserInputData = inputData;
			}

			mtx_lock.unlock();
		}

		bool clear_cin()
		{
			using namespace std;
			if (cin.fail())
			{
				cin.clear();
				cin.ignore(256, '\n');
				cout << "input error." << endl;
				return true;
			}
			return false;
		}

		void Loop()
		{
			using namespace std;
			cout << "=====AsyncConsoleIO=====" << endl;
			cout << "input: Key Value" << endl;
			//cout << "Key: !keylist" << endl;
			cout << "=====AsyncConsoleIO=====" << endl;

			while (m_bLoop)
			{
				cout << "Key: ";
				string key;
				cin >> key;
				if (clear_cin()) continue;

				auto iter = m_mapOrder.find(key);
				if (iter != m_mapOrder.end())
				{
					ACIOData & tACIOData = (*iter).second;

					switch (tACIOData.eKeyType)
					{
					case EKeyType::Char:
					{
						user_input<unsigned char>(tACIOData);
						break;
					}

					case EKeyType::Word:
					{
						user_input<WORD>(tACIOData);
						break;
					}

					case EKeyType::Int:
					{
						user_input<int>(tACIOData);
						break;
					}

					case EKeyType::Float:
					{
						user_input<float>(tACIOData);
						break;
					}

					case EKeyType::Double:
					{
						user_input<double>(tACIOData);
						break;
					}

					case EKeyType::Int64:
					{
						user_input<__int64>(tACIOData);
						break;
					}

					default:
						assert(false);
						break;
					}
				}
			}
		}


		AsyncConsoleIO()
		{
#ifdef USE_AsyncConsoleIO
			if (!(m_hThread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, &m_dwThreadId)))
			{
				MessageBox(NULL,
					(LPCSTR)L"CreateThread failed.",
					(LPCSTR)L"AsyncConsoleIO", MB_OK);
				ExitProcess(3);
			}
#endif
		}

		~AsyncConsoleIO()
		{
#ifdef USE_AsyncConsoleIO
			if (m_hThread)
			{
				m_bLoop = false;
				WaitForSingleObject(m_hThread, INFINITE);

				CloseHandle(m_hThread);
				m_hThread = nullptr;
				m_dwThreadId = 0;

				auto iter = m_mapOrder.begin();
				auto iterEnd = m_mapOrder.end();

				for (; iter != iterEnd; ++iter)
				{
					delete iter->second.ppUserInputData;
					iter->second.ppUserInputData = nullptr;
				}

				m_mapOrder.clear();
				g_AsyncConsoleIO = nullptr;
			}
#endif
		}
	};
}
