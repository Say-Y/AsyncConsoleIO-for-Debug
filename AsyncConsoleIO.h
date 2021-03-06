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
#include <string>

namespace ACIO
{
	enum class EKeyType
	{
		Bool,
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

			ACIOData(void ** _ppData, void ** _ppUserInputData, EKeyType _eKeyType, bool _bForced)
				: ppData(_ppData), ppUserInputData(_ppUserInputData), eKeyType(_eKeyType), bForced(_bForced)
			{
			}
		};

	private:
		std::hash<std::string> m_Hasher;

		std::mutex	mtx_lock = {};
		HANDLE		m_hThread = nullptr;
		DWORD		m_dwThreadId = 0;
		std::map<int, ACIOData> m_mapOrder;

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
#ifdef USE_AsyncConsoleIO
			if (!m_hThread) assert(false);
			//if (key.size() == 0) assert(false);

			//set_lock();
			size_t hashedKey = m_Hasher(key);
			auto iter = m_mapOrder.find(hashedKey);
			if (iter == m_mapOrder.end())
			{
				m_mapOrder.insert(std::make_pair(hashedKey, ACIOData( ppValue, nullptr, eKeyType, false )));
			}
			else
			{
				if ((*iter).second.ppData != ppValue)
					assert(false && "bind_data: It is different from the address previously registered.");

				if ((*iter).second.bForced)
					assert(false && "bind_data: Do not use with bind_data_forced.");

				if ((*iter).second.eKeyType != eKeyType)
					assert(false && "bind_data: Type is different.");

				ACIOData data{ ppValue, nullptr, eKeyType, false };
				switch ((*iter).second.eKeyType)
				{
				case EKeyType::Bool:
				{
					input_forced<bool>(data, **(bool **)ppValue);
					break;
				}

				case EKeyType::Char:
				{
					input_forced<unsigned char>(data, **(unsigned char**)ppValue);
					break;
				}

				case EKeyType::Word:
				{
					input_forced<WORD>(data, **(WORD**)ppValue);
					break;
				}

				case EKeyType::Int:
				{
					input_forced<int>(data, **(int**)ppValue);
					break;
				}

				case EKeyType::Float:
				{
					input_forced<float>(data, **(float**)ppValue);
					break;
				}

				case EKeyType::Double:
				{
					input_forced<double>(data, **(double**)ppValue);
					break;
				}

				case EKeyType::Int64:
				{
					input_forced<__int64>(data, **(__int64**)ppValue);
					break;
				}

				default:
					assert(false);
					break;
				}
			}

			//set_unlock();
#endif
		}

		/// <summary> It is overwritten with the value entered at the console at the time of the call.</summary>
		/// <param name="key"> Key to enter in console. </param>
		/// <param name="ppValue"> Address of the variable. </param>
		/// <param name="iTypeSize"> Must be one of 1,2,3,4,8 </param>
		void bind_data_forced(std::string key, void ** ppValue, EKeyType eKeyType)
		{
#ifdef USE_AsyncConsoleIO
			if (!m_hThread) assert(false);
			//if (key.size() == 0) assert(false);

			//set_lock();
			size_t hashedKey = m_Hasher(key);
			auto iter = m_mapOrder.find(hashedKey);
			if (iter == m_mapOrder.end())
			{
				m_mapOrder.insert(std::make_pair(hashedKey, ACIOData{ ppValue, nullptr, eKeyType, true }));
			}
			else
			{
				if ((*iter).second.ppData != ppValue)
					assert(false && "bind_data_forced: It is different from the address previously registered.");

				if (!(*iter).second.bForced)
					assert(false && "bind_data_forced: Do not use with bind_data.");

				if ((*iter).second.eKeyType != eKeyType)
					assert(false && "bind_data_forced: Type is different.");

				if ((*iter).second.ppUserInputData)
				{
					switch ((*iter).second.eKeyType)
					{
					case EKeyType::Bool:
					{
						bool * pptr = (bool *)(*iter).second.ppUserInputData;
						input_forced<bool>((*iter).second, (bool)*pptr);
						break;
					}

					case EKeyType::Char:
					{
						char * pptr = (char *)(*iter).second.ppUserInputData;
						input_forced<char>((*iter).second, (char)*pptr);
						break;
					}

					case EKeyType::Word:
					{
						WORD * pptr = (WORD *)(*iter).second.ppUserInputData;
						input_forced<WORD>((*iter).second, (WORD)*pptr);
						break;
					}

					case EKeyType::Int:
					{
						int * pptr = (int *)(*iter).second.ppUserInputData;
						input_forced<int>((*iter).second, (int)*pptr);
						break;
					}

					case EKeyType::Float:
					{
						float * pptr = (float *)(*iter).second.ppUserInputData;
						input_forced<float>((*iter).second, (float)*pptr);
						break;
					}

					case EKeyType::Double:
					{
						double * pptr = (double *)(*iter).second.ppUserInputData;
						input_forced<double>((*iter).second, (double)*pptr);
						break;
					}

					case EKeyType::Int64:
					{
						__int64 * pptr = (__int64 *)(*iter).second.ppUserInputData;
						input_forced<__int64>((*iter).second, (__int64)*pptr);
						break;
					}

					default:
						assert(false);
						break;
					}
				}
			}

			//set_unlock();
#endif
		}

		static AsyncConsoleIO * GetInst()
		{
			if (!g_AsyncConsoleIO) g_AsyncConsoleIO = new AsyncConsoleIO;

			return g_AsyncConsoleIO;
		}

		void DestroyInst()
		{
			delete g_AsyncConsoleIO;
		}

	private:
		template <typename T>
		void input_forced(ACIOData & tACIOData, T _inputData)
		{
			if (tACIOData.bForced)
			{
				if (!tACIOData.ppUserInputData)
				{
					T* tUserInputData = new T;
					tACIOData.ppUserInputData = (void**)tUserInputData;
				}

				T* pUserInputData = (T*)tACIOData.ppData;
				*pUserInputData = _inputData;
			}
		}

		template <typename T>
		void user_input(ACIOData & tACIOData)
		{
			using namespace std;
			T inputData = 0;
			cin >> inputData;
			bool bFfaled = clear_cin();
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
				else if (bFfaled)
				{
					delete tACIOData.ppUserInputData;
					tACIOData.ppUserInputData = nullptr;
				}

				if (tACIOData.ppUserInputData)
				{
					T* pUserInputData = (T*)tACIOData.ppUserInputData;
					*pUserInputData = inputData;
				}
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
				size_t hashedKey = m_Hasher(key);
				auto iter = m_mapOrder.find(hashedKey);
				if (iter != m_mapOrder.end())
				{
					ACIOData & tACIOData = (*iter).second;

					switch (tACIOData.eKeyType)
					{
					case EKeyType::Bool:
					{
						user_input<bool>(tACIOData);
						break;
					}

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
				assert(false);
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