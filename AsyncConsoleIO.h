#pragma once
// AsyncConsoleIO V.1

// Copyright (c) <2019> <Hani Kim(Kim Han Byeol)>
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
#include <typeinfo>
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

	/// Do not write in both directions. NOT SAFE.
	class AsyncConsoleIO
	{
	private:
		struct ACIOData
		{
			EKeyType eKeyType = EKeyType::End;
			void** ppData = nullptr;
		};

	private:
		HANDLE  m_hThread = nullptr;
		DWORD   m_dwThreadId = 0;
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
		
		/// <summary> Replaces an existing pointer if a duplicate key exists. </summary>
		/// <param name="key"> Key </param>
		/// <param name="ppValue"> Address of the variable. </param>
		/// <param name="iTypeSize"> Must be one of 1,2,3,4,8 </param>
		void bind_data(std::string key, void ** ppValue, EKeyType eKeyType)
		{
			if (key.size() == 0) assert(false);

			auto iter = m_mapOrder.find(key);
			if (iter == m_mapOrder.end())
			{
				m_mapOrder.insert(make_pair(key, ACIOData{ eKeyType, ppValue }));
			}
			else
			{
				(*iter).second.eKeyType = eKeyType;
				(*iter).second.ppData = ppValue;
			}
		}

		static AsyncConsoleIO * GetInst()
		{
			static AsyncConsoleIO instance;
			if (!instance.m_hThread)
				assert(false);

			return &instance;
		}

		void DestroyInst()
		{
			this->~AsyncConsoleIO();
		}

	private:
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
				cin.clear();
				cin.ignore(256, '\n');

				cout << "Key: ";
				string key;
				cin >> key;
				if (clear_cin()) continue;

				auto iter = m_mapOrder.find(key);
				if (iter != m_mapOrder.end())
				{
					ACIOData tACIOData = (*iter).second;

					switch (tACIOData.eKeyType)
					{
					case EKeyType::Char:
					{
						unsigned char inputData = 0;
						cin >> inputData;

						unsigned char* pData = (unsigned char*)tACIOData.ppData;
						*pData = inputData;
						break;
					}

					case EKeyType::Word:
					{
						WORD inputData = 0;
						cin >> inputData;

						WORD* pData = (WORD*)tACIOData.ppData;
						*pData = inputData;
						break;
					}

					case EKeyType::Int:
					{
						UINT inputData = 0;
						cin >> inputData;

						UINT* pData = (UINT*)tACIOData.ppData;
						*pData = inputData;
						break;
					}

					case EKeyType::Float:
					{
						float inputData = 0.f;
						cin >> inputData;

						float* pData = (float*)tACIOData.ppData;
						*pData = inputData;
						break;
					}

					case EKeyType::Double:
					{
						double inputData = 0;
						cin >> inputData;

						double* pData = (double*)tACIOData.ppData;
						*pData = inputData;
						break;
					}

					case EKeyType::Int64:
					{
						unsigned __int64 inputData = 0;
						cin >> inputData;

						unsigned __int64* pData = (unsigned __int64*)tACIOData.ppData;
						*pData = inputData;
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
			}
#endif
		}
	};
}
