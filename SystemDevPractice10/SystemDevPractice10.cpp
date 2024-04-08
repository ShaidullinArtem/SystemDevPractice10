#include <iostream>
#include <windows.h>
using namespace std;

int Task1() {
	// Инициализация структуры STARTUPINFO
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	// Инициализация структуры PROCESS_INFORMATION
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	// Путь к исполняемому файлу cmd.exe
	wchar_t cmdPath[] = L"ConsoleProcess1.exe";
	
	// Создание нового процесса
	if (!CreateProcess(
		NULL,
		cmdPath,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&si,
		&pi
	)) {
		cerr << "CreateProcess failed (" << GetLastError() << ")." << endl;
		return 1;
	}

	// Вывод PID и дескриптора процесса
	cout << "Process ID: " << pi.dwProcessId << endl;
	cout << "Process Handle: " << pi.hProcess << endl;
	
	// Закрытие дескрипторов процесса и потока
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

	return 0;

}

int Task2() {
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	wchar_t cmdPath[] = L"ConsoleProcess1.exe";

	if (!CreateProcess(
		NULL,
		cmdPath,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&si,
		&pi
	)) {
		cerr << "CreateProcess failed (" << GetLastError() << ")." << endl;
	}

	cout << "Process ID: " << pi.dwProcessId << endl;
	cout << "Process Handle: " << pi.hProcess << endl;

	if (!TerminateProcess(pi.hProcess, 0)) {
		cerr << "TerminateProcess failed (" << GetLastError() << ")." << endl;
		return 1;
	}

	// Вывод сообщения о завершении процесса
	std::cout << "Process terminated successfully.\n";

	// Закрытие дескрипторов процесса и потока
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return 0;
}

int Task3() {
	HANDLE hRead, hWrite;

	// Создаем анонимный канал
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
		cerr << "Create Pipe error!" << endl;
		return 1;
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;
	si.dwFlags |= STARTF_USESTDHANDLES;
	wchar_t commandLine[] = L"ConsoleProcess1.exe";

	if (!CreateProcess(NULL, commandLine, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
		cerr << "Create Process error!" << endl;
		return 1;
	}

	// Закрываем записывающий конец канала в родительском процессе
	CloseHandle(hWrite);

	// Читаем данные из канала
	char buffer[32];
	memset(buffer, 0, sizeof(buffer));
	DWORD dwRead;
	bool bSuccess = ReadFile(hRead, buffer, sizeof(buffer), &dwRead, NULL);
	if (!bSuccess || dwRead == 0) {
		cerr << "File Read error" << endl;
		return 1;
	}

	// Выводим прочитанные данные на экран
	cout << buffer << endl;

	// Закрываем дескрипторы
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return 0;
}

int Task4() {
	LPCWSTR filename = L"test.txt";
	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
		cerr << "File open error!" << filename << endl;
		return 1;
	}

	// Создаем копию дескриптора файла
	HANDLE hDuplicateFile;
	if (!DuplicateHandle(
		GetCurrentProcess(), 
		hFile, GetCurrentProcess(), 
		&hDuplicateFile, 
		0, 
		FALSE, 
		DUPLICATE_SAME_ACCESS
	)) {
		cerr << "Descriptor duplicate error!" << std::endl;
		CloseHandle(hFile);
		return 1;
	}

	char buffer[1024];
	DWORD bytesRead;

	// Перемещаем указатель файла в начало
	SetFilePointer(hDuplicateFile, 0, NULL, FILE_BEGIN);

	while (ReadFile(hDuplicateFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
		buffer[bytesRead] = '\0';
		cout << buffer;
	}

	CloseHandle(hFile);
	CloseHandle(hDuplicateFile);

	return 0;
}

int Task5() {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	wchar_t commandLine[] = L"ConsoleProcess1.exe";

	// Создаем новый процесс
	if (!CreateProcess(NULL, commandLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
		cerr << "Create Process error! " << GetLastError() << endl;
		return 1;
	}

	// Получаем псевдодескриптор текущего процесса
	HANDLE pseudoHandle = GetCurrentProcess();

	cout << "Process " << pseudoHandle << endl;

	// Выделяем память в другом процессе
	LPVOID remoteMemory = VirtualAllocEx(pi.hProcess, NULL, sizeof(HANDLE), MEM_COMMIT, PAGE_READWRITE);
	if (remoteMemory == NULL) {
		cerr << "Memory error! " << GetLastError() << endl;
		return 1;
	}

	// Записываем псевдодескриптор в память другого процесса
	SIZE_T bytesWritten;
	if (!WriteProcessMemory(pi.hProcess, remoteMemory, &pseudoHandle, sizeof(HANDLE), &bytesWritten)) {
		cerr << "Process Write Memory error!" << GetLastError() << endl;
		return 1;
	}

	// Создаем удаленный поток в другом процессе, который выводит псевдодескриптор в консоль
	HANDLE hThread = CreateRemoteThread(pi.hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)remoteMemory, NULL, 0, NULL);
	if (hThread == NULL) {
		cerr << "Create Thread error!" << GetLastError() << endl;
		return 1;
	}

	DWORD currentProcessId = GetCurrentProcessId();
	HANDLE realHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, currentProcessId);

	cout << "Current Process: " << realHandle << endl;

	// Закрываем дескрипторы процесса и потока
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return 0;
}


// Task 6 start
DWORD WINAPI AsyncEvenSumThread(LPVOID param) {
	if (param == NULL) {
		cerr << "Invalid Params!";
		return 1;
	}

	int lim = *reinterpret_cast<int*>(param);
	int result = 0;

	printf("Calculating even sum from 1 to %d\n", lim);
	for (int i = 0; i <= lim; i++)
	{
		if (i % 2 == 0) result += i;
	}

	printf("\nResult: %d\n", result);

	return 0;
}

DWORD WINAPI AsyncBubbleSort(LPVOID param) {
	if (param == NULL) {
		cerr << "Invalid Params!";
		return 1;
	}

	int* arr = reinterpret_cast<int*>(param);
	int size = arr[0];

	cout << "Initial array:" << endl;
	for (int i = 0; i < size; i++)
	{
		cout << arr[i] << "\t";
	}

	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			if (arr[i] < arr[j]) {
				int buf = arr[i];
				arr[i] = arr[j];
				arr[j] = buf;
			}
		}
	}
	cout << "\nResult:" << endl;
	for (int i = 0; i < size; i++)
	{
		cout << arr[i] << "\t";
	}

	return 0;
}

DWORD WINAPI AsyncStringReverse(LPVOID param) {
	if (param == NULL) {
		cerr << "Invalid params!";
		return 1;
	}

	char* string = reinterpret_cast<char*>(param);
	int size = string[0];

	cout << "Result:" << endl;
	for (int i = size; i >= 0; i--)
	{
		cout << string[i];
	}

	return 0;
}

void Task6() {

	int evenLim = 10;
	int bubbleSortArr[] = { 4, 2, 1, 5, 2, 56 };
	char reverseString[] = "hello";

	HANDLE hThread1 = CreateThread(NULL, 0, AsyncEvenSumThread, &evenLim, 0, NULL);
	HANDLE hThread2 = CreateThread(NULL, 0, AsyncBubbleSort, &bubbleSortArr, 0, NULL);
	HANDLE hThread3 = CreateThread(NULL, 0, AsyncStringReverse, &reverseString, 0, NULL);

	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);
	WaitForSingleObject(hThread3, INFINITE);

	CloseHandle(hThread1);
	CloseHandle(hThread2);
	CloseHandle(hThread3);
}

// Task 6 end

int Task7() {

	const short MAX_THREADS = 10;
	int evenLim = 10;

	HANDLE threads[MAX_THREADS];
	int threadIds[MAX_THREADS];
	int threadCount = 0;

	while (true) {
		cout << "1. Create THREAD\n";
		cout << "2. Close THREAD\n";
		cout << "3. Exit\n";
		cout << "Choose action: ";
		int choice;
		cin >> choice;

		switch (choice) {
		case 1: {
			// Создание потока
			if (threadCount >= MAX_THREADS) {
				cout << "Max THREADS.\n";
				break;
			}
			threadIds[threadCount] = threadCount;
			threads[threadCount] = CreateThread(NULL, 0, AsyncEvenSumThread, &evenLim, 0, NULL);
			++threadCount;
			break;
		}
		case 2: {
			// Завершение потока
			cout << "Enter THEARD ID: ";
			int threadId;
			cin >> threadId;
			if (threadId < 0 || threadId >= threadCount) {
				cout << "Undefind THEARD ID: " << threadId << endl;
			}
			else {
				WaitForSingleObject(threads[threadId], INFINITE);
				CloseHandle(threads[threadId]);
				cout << "THEARD " << threadId << " closed.\n";

				for (int i = threadId; i < threadCount - 1; ++i) {
					threads[i] = threads[i + 1];
					threadIds[i] = threadIds[i + 1];
				}
				--threadCount;
			}
			break;
		}
		case 3:
			return 0;
		default:
			cout << "Invalid action!";
		}
	}

	return 0;
}

int main()
{
	unsigned short action;
	
	cout << "Enter task number (1 - 7):" << endl;
	cin >> action;

	switch (action)
	{
	case 1:
		Task1();
		break;
	case 2:
		Task2();
		break;
	case 3:
		Task3();
		break;
	case 4:
		Task4();
		break;
	case 5:
		Task5();
		break;
	case 6:
		Task6();
		break;
	case 7:
		Task7();
		break;
	default:
		cout << "Undefind action" << endl;
		break;
	}

	return 0;
}


