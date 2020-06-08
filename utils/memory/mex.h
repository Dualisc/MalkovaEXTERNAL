//Every parameter of 'uintptr_t' type refers to an address in the context of the virtual address space of the opened process.
//Every parameter of pointer type refer to an address/pointer/buffer in the context of the virtual address space of the current process.

#ifndef MEMEX_H
#define MEMEX_H

#include <Windows.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <atomic>
#include <string>
#include <TlHelp32.h>
#include <memory>
#include <thread>
#include <algorithm>

#define HOOK_MARK_END __asm _emit 0xD6 __asm _emit 0xD6 __asm _emit 0x0F __asm _emit 0x0F __asm _emit 0x0F __asm _emit 0x0F __asm _emit 0x0F __asm _emit 0x0F __asm _emit 0x0F __asm _emit 0x0F __asm _emit 0xD6 __asm _emit 0xD6

#define CPTR(pointerToData, sizeOfData) ArgPtr(pointerToData, sizeOfData, true, false)
#define PTR(pointerToData, sizeOfData) ArgPtr(pointerToData, sizeOfData, false, false)

enum class SCAN_BOUNDARIES
{
	RANGE,
	MODULE,
	ALL_MODULES
};

struct ScanBoundaries
{
	const SCAN_BOUNDARIES scanBoundaries;
	union
	{
		struct { uintptr_t start, end; };
		const TCHAR* const moduleName;
	};

	ScanBoundaries(const SCAN_BOUNDARIES scanBoundaries, const uintptr_t start, const uintptr_t end);
	ScanBoundaries(const SCAN_BOUNDARIES scanBoundaries, const TCHAR* const moduleName);
	ScanBoundaries(const SCAN_BOUNDARIES scanBoundaries);
};


enum class SCAN_TYPE
{
	EXACT,
	BETWEEN,
	GREATER_THAN,
	LESS_THAN,
	UNCHANGED,
	CHANGED,
	INCREASED,
	INCREASED_BY,
	DECREASED,
	DECREASED_BY,
	UNKNOWN
};

enum class VALUE_TYPE
{
	ONE_BYTE = 0x01,
	TWO_BYTES = 0x02,
	FOUR_BYTES = 0x04,
	EIGTH_BYTES = 0x08,
	FLOAT = 0x10,
	DOUBLE = 0x20,
	ALL = (ONE_BYTE | TWO_BYTES | FOUR_BYTES | EIGTH_BYTES | FLOAT | DOUBLE)
};

enum class FLOAT_ROUNDING
{
	NONE,
	ROUND,
	TRUNCATE
};

template<typename T>
struct Scan
{
	SCAN_TYPE scanType;

	T value, value2;
	FLOAT_ROUNDING floatRounding;

	Scan(const SCAN_TYPE scanType, T value, const FLOAT_ROUNDING floatRounding = FLOAT_ROUNDING::NONE)
		: scanType(scanType),
		value(value),
		value2(T()),
		floatRounding(floatRounding) {}

	Scan(const SCAN_TYPE scanType, T value, T value2, const FLOAT_ROUNDING floatRounding = FLOAT_ROUNDING::NONE)
		: scanType(scanType),
		value(value),
		value2(value2),
		floatRounding(floatRounding) {}

	Scan(const SCAN_TYPE scanType)
		: scanType(scanType),
		value(T()),
		value2(T()),
		floatRounding(FLOAT_ROUNDING::NONE) {}
};

template<typename T>
struct Value
{
	uintptr_t address;
	T value;

	Value(uintptr_t address, T& value)
		: address(address),
		value(value) {}

	//For use in std::sort()
	bool operator<(const Value& other) const { return address < other.address; }
};

struct AOB
{
	char* aob;
	size_t size;

	AOB() : aob(nullptr), size(0) {}

	AOB(const char* _aob) : aob(const_cast<char*>(_aob)), size(0) {}

	AOB(const AOB& other)
		:size(other.size)
	{
		if (size)
		{
			aob = new char[size];
			memcpy(aob, other.aob, size);
		}
		else
			aob = other.aob;
	}

	AOB(const char* _aob, size_t size)
		: size(size)
	{
		aob = new char[size];
		memcpy(aob, _aob, size);
	}

	~AOB()
	{
		if (size)
			delete[] aob;
	}
};

template<typename T>
struct ThreadData
{
	std::thread thread;
	std::vector<Value<T>> values;
};

enum class INJECTION_METHOD
{
	LOAD_LIBRARY,
	MANUAL_MAPPING
};

typedef struct ArgPtr
{
	const void* const data;
	const size_t size;
	const bool constant, immediate, isString;
	void* volatileBuffer;

#ifdef _WIN64
	bool isFloat = false;
#endif

	ArgPtr(const void* pointerToData, const size_t sizeOfData, const bool isDataConstant = true, const bool isDataImmediate = false, const bool isDataString = false)
		:data(pointerToData),
		size(sizeOfData),
		constant(isDataConstant),
		immediate(isDataImmediate),
		isString(isDataString),
		volatileBuffer(nullptr) {}
} Arg;

//List of suported calling conventions
enum class CConv
{
	DEFAULT,
	THIS_PTR_RET_SIZE_OVER_8,
#ifndef _WIN64
	_CDECL,
	//_CLRCALL, Only callable from managed code.
	_STDCALL,
	_FASTCALL,
	_THISCALL,
	//_VECTORCALL, [TODO]
#endif
};

//CPU STATES(for use in the saveCpuStateMask parameter on the Hook() function)
#define GPR 0x01
#define FLAGS 0x02
#define XMMX 0x04

class MemEx
{
	struct Nop
	{
		std::unique_ptr<uint8_t[]> buffer;
		size_t size = 0;
	};

	struct HookStruct
	{
		uintptr_t buffer = 0;
		uint8_t bufferSize = 0;
		uint8_t numReplacedBytes = 0;
		bool useCodeCaveAsMemory = true;
		uint8_t codeCaveNullByte = 0;
	};

	HANDLE m_hProcess; // A handle to the target process.
	DWORD m_dwProcessId; // The process id of the target process.

	HANDLE m_hFileMapping; // A handle to the file mapping object.
	HANDLE m_hFileMappingDuplicate; // A handle to the file mapping object valid on the target process. In case the system doesn't support MapViewOfFile2.
	uint8_t* m_thisMappedView; // Starting address of the mapped view on this process.
	uint8_t* m_targetMappedView; // Starting address of the mapped view on the target process.

	size_t m_numPages;

	//Event objects to perform synchronization with our thread on the target process.
	HANDLE m_hEvent1, m_hEventDuplicate1;
	HANDLE m_hEvent2, m_hEventDuplicate2;

	HANDLE m_hThread; // A handle to our thread on the target process.

	//Store addresses/bytes which the user nopped so they can be restored later with Patch()
	std::unordered_map<uintptr_t, Nop> m_Nops;

	//Key(uintptr_t) stores the address of the hooked function
	std::map<uintptr_t, HookStruct> m_Hooks;

	std::unordered_map<uintptr_t, size_t> m_Pages;

	bool m_isWow64;
public:
	const static DWORD dwPageSize;
	const static DWORD dwDesiredAccess;

	MemEx();
	~MemEx();

	//Returns true if opened, false otherwise.
	bool IsOpened();

	//Opens to a process using a handle.
	//Parameters:
	//  hProcess [in] A handle to the process. The handle must have the following permissions:
	//                PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION. If Hook() or
	//                Call() is used, the handle must also have the following permissions:
	//                PROCESS_DUP_HANDLE | PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION.
	bool Open(const HANDLE hProcess);

	//Opens to a process using a PID.
	//Parameters:
	//  dwProcessId     [in] The process's id.
	//  dwDesiredAccess [in] The access for the process handle.
	bool Open(const DWORD dwProcessId, const DWORD dwDesiredAccess = MemEx::dwDesiredAccess);

	//Opens to a process using its name.
	//Parameters:
	//  processName     [in] The process's name.
	//  dwDesiredAccess [in] The access for the process handle.
	bool Open(const TCHAR* const processName, const DWORD dwDesiredAccess = MemEx::dwDesiredAccess);

	//Opens to a process using a window and class name.
	//Parameters:
	//  windowName      [in] The window's title. If NULL, all window 
	//                       names match.
	//  className       [in] The class name. If NULL, any window title
	//                       matching windowName is considered.
	//  dwDesiredAccess [in] The access for the process handle.
	bool OpenByWindow(const TCHAR* const windowName, const TCHAR* const className = nullptr, const DWORD dwDesiredAccess = MemEx::dwDesiredAccess);

	//Opens to a process using its name. The functions does not return until a process that matches processName is found.
	//Parameters:
	//  processName     [in] The process's name.
	//  dwDesiredAccess [in] The access for the process handle.
	//  dwMilliseconds  [in] The number of milliseconds the
	//                       thread sleeps every iteration.
	void WaitOpen(const TCHAR* const processName, const DWORD dwDesiredAccess = MemEx::dwDesiredAccess, const DWORD dwMilliseconds = 500);

	//Opens to a process using a window and class name. The functions does not return until a process that matches processName is found.
	//Parameters:
	//  windowName      [in] The window's title. If NULL, all window 
	//                       names match.
	//  className       [in] The class name. If NULL, any window title
	//                       matching windowName is considered.
	//  dwDesiredAccess [in] The access for the process handle.
	//  dwMilliseconds  [in] The number of milliseconds the thread 
	//                       sleeps every iteration.
	void WaitOpenByWindow(const TCHAR* const windowName, const TCHAR* const className = nullptr, const DWORD dwDesiredAccess = MemEx::dwDesiredAccess, const DWORD dwMilliseconds = 500);

	//Terminates any remote threads and memory allocations created by this library on the process. 
	void Close();

	//Retuns a handle to the opened process.
	HANDLE GetProcess() const;

	//Returns the PID of the opened process.
	DWORD GetPid() const;

	//Returns a copy of the data at 'address'.
	//Parameters:
	//  address [in]  The address where the bytes will be read from.
	template <typename T>
	inline T Read(const uintptr_t address) const
	{
		T t;
		if (!Read(address, &t, sizeof(T)))
			memset(&t, 0x00, sizeof(T));
		return t;
	}

	//Copies 'size' bytes from 'address' to 'buffer'.
	//Parameters:
	//  address [in]  The address where the bytes will be copied from.
	//  buffer  [out] The buffer where the bytes will be copied to.
	//  size    [in]  The number of bytes to be copied.
	bool Read(const uintptr_t address, void* const buffer, const SIZE_T size) const;

	//Copies 'value' to 'address'.
	//Parameters:
	//  address [in] The address where the bytes will be copied to.
	//  value   [in] The value where the bytes will be copied from.
	template <typename T>
	inline bool Write(uintptr_t address, const T& value) const { return Write(address, &value, sizeof(T)); }

	//Copies 'size' bytes from 'buffer' to 'address'.
	//Parameters:
	//  address [in] The address where the bytes will be copied to.
	//  buffer  [in] The buffer where the bytes will be copied from.
	//  size    [in] The number of bytes to be copied.
	bool Write(uintptr_t address, const void* const buffer, const SIZE_T size) const;

	//Patches 'address' with 'size' bytes stored on 'buffer'.
	//Parameters:
	//  address [in] The address where the bytes will be copied to.
	//  buffer  [in] The buffer where the bytes will be copied from.
	//  size    [in] The number of bytes to be copied.
	bool Patch(const uintptr_t address, const char* const bytes, const size_t size) const;

	//Writes 'size' 0x90(opcode for the NOP(no operation) instruction) bytes at address.
	//Parameters:
	//  address   [in] The address where the bytes will be nopped.
	//  size      [in] The number of bytes to be written.
	//  saveBytes [in] If true, save the original bytes located at 'address'
	//                 where they can be later restored by calling Restore().
	bool Nop(const uintptr_t address, const size_t size, const bool saveBytes = true);

	//Restores the bytes that were nopped at 'address'.
	//Parameters:
	//  address   [in] The address where the bytes will be restored.
	bool Restore(const uintptr_t address);

	//Copies 'size' bytes from 'sourceAddress' to 'destinationAddress'.
	//Parameters:
	//  destinationAddress [in] The destination buffer's address.
	//  sourceAddress      [in] The souce buffer's address.
	//  size               [in] The number of bytes to be copied.
	bool Copy(const uintptr_t destinationAddress, const uintptr_t sourceAddress, const size_t size) const;

	//Sets 'size' 'value' bytes at 'address'.
	//Parameters:
	//  address [in] The address where the bytes will be written to.
	//  value   [in] The byte to be set.
	//  size    [in] The nmber of bytes to be set.
	bool Set(const uintptr_t address, const int value, const size_t size) const;

	//Compares the first 'size' bytes of 'address1' and 'address2'.
	//Parameters:
	//  address1 [in] the address where the first buffer is located.
	//  address2 [in] the address where the second buffer is located.
	//  size     [in] The number of bytes to be compared.
	bool Compare(const uintptr_t address1, const uintptr_t address2, const size_t size) const;

	//Calculates the MD5 hash of a memory region of the opened process.
	//Parameters:
	//  address [in]  The address where the hash will be calculated.
	//  size    [in]  The size of the region.
	//  outHash [out] A buffer capable of holding a MD5 hash which is 16 bytes.
	bool HashMD5(const uintptr_t address, const size_t size, uint8_t* const outHash) const;

	//Scans the address space according to 'scanBoundaries' for a pattern & mask.
	//Parameters:
	//  pattern        [in] A buffer containing the pattern. An example of a
	//                      pattern is "\x68\xAB\x00\x00\x00\x00\x4F\x90\x00\x08".
	//  mask           [in] A string that specifies how the pattern should be 
	//                      interpreted. If mask[i] is equal to '?', then the
	//                      byte pattern[i] is ignored. A example of a mask is
	//                      "xx????xxxx".
	//  scanBoundaries [in] See definition of the ScanBoundaries class.
	//  protect        [in] Specifies a mask of memory protection constants
	//                      which defines what memory regions will be scanned.
	//                      The default value(-1) specifies that pages with any
	//                      protection between 'start' and 'end' should be scanned.
	//  numThreads     [in] The number of threads to be used. Thr default argument
	//                      uses the number of CPU cores as the number of threads.
	//  firstMatch     [in] If true, the address returned(if any) is guaranteed to
	//                      be the first match(i.e. the lowest address on the virtual
	//                      address space that is a match) according to scanBoundaries.
	uintptr_t PatternScan(const char* const pattern, const char* const mask, const ScanBoundaries& scanBoundaries = ScanBoundaries(SCAN_BOUNDARIES::RANGE, 0, -1), const DWORD protect = -1, const size_t numThreads = static_cast<size_t>(std::thread::hardware_concurrency()), const bool firstMatch = false) const;

	//Scans the address space according to 'scanBoundaries' for an AOB.
	//Parameters:
	//  AOB            [in]  The array of bytes(AOB) in string form. To specify
	//                       a byte that should be ignore use the '?' character.
	//                       An example of AOB is "68 AB ?? ?? ?? ?? 4F 90 00 08".
	//  scanBoundaries [in]  See definition of the ScanBoundaries class.
	//  protect        [in]  Specifies a mask of memory protection constants
	//                       which defines what memory regions will be scanned.
	//                       The default value(-1) specifies that pages with any
	//                       protection between 'start' and 'end' should be scanned.
	//  patternSize    [out] A pointer to a variable that receives the size of the
	//                       size of the pattern in bytes. This parameter can be NULL.
	//  numThreads     [in]  The number of threads to be used. Thr default argument
	//                       uses the number of CPU cores as the number of threads.
	//  firstMatch     [in]  If true, the address returned(if any) is guaranteed to
	//                       be the first match(i.e. the lowest address on the virtual
	//                       address space that is a match) according to scanBoundaries.
	uintptr_t AOBScan(const char* const AOB, const ScanBoundaries& scanBoundaries = ScanBoundaries(SCAN_BOUNDARIES::RANGE, 0, -1), const DWORD protect = -1, size_t* const patternSize = nullptr, const size_t numThreads = static_cast<size_t>(std::thread::hardware_concurrency()), const bool firstMatch = false) const;

	//Reads a multilevel pointer.
	//Parameters:
	//  base    [in] The base address.
	//  offsets [in] A vector specifying the offsets.
	uintptr_t ReadMultiLevelPointer(const uintptr_t base, const std::vector<uint32_t>& offsets) const;

	//Do not use 'void' as return type, use any other type instead.
	template<typename TyRet = int, CConv cConv = CConv::DEFAULT, typename ... Args>
	TyRet Call(const uintptr_t address, Args&& ... arguments)
	{
		if ((!m_hThread && !SetupRemoteThread()) || (cConv == CConv::THIS_PTR_RET_SIZE_OVER_8 && sizeof(TyRet) <= 8))
			return TyRet();

		//Parse arguments
		std::vector<Arg> args;
		GetArguments(args, arguments...);

		return *static_cast<TyRet*>(CallImpl(cConv, std::is_same<TyRet, float>::value, std::is_same<TyRet, double>::value, sizeof(TyRet), address, args));
	}

	//Hooks an address. You must use the HOOK_MARK_END macro.
	//Parameters:
	//  address          [in]  The address to be hooked.
	//  callback         [in]  The callback to be executed when the CPU executes 'address'.
	//  trampoline       [out] An optional pointer to a variable that receives the address
	//                         of the trampoline. The trampoline contains the original replaced
	//                         instructions of the 'address' and a jump back to 'address'.
	//  saveCpuStateMask [in]  A mask containing a bitwise OR combination of one or more of
	//                         the following macros: GPR(general purpose registers),
	//                         FLAGS(eflags/rflags), XMMX(xmm0, xmm1, xmm2, xmm3, xmm4, xmm5).
	//                         Push the CPU above states to the stack before executing callback.
	//                         You should use this parameter if you perform a mid function hook.
	//                         By default no CPU state is saved.
	bool Hook(const uintptr_t address, const void* const callback, uintptr_t* const trampoline = nullptr, const DWORD saveCpuStateMask = 0);

	//Hooks an address by passing a buffer with known size at compile time as the callback.
	//Parameters:
	//  address                [in]  The address to be hooked.
	//  callback[callbackSize] [in]  The callback to be executed when the CPU executes 'address'.
	//  trampoline             [out] An optional pointer to a variable that receives the address
	//                               of the trampoline. The trampoline contains the original replaced
	//                               instructions of the 'address' and a jump back to 'address'.
	//  saveCpuStateMask       [in]  A mask containing a bitwise OR combination of one or more of
	//                               the following macros: GPR(general purpose registers),
	//                               FLAGS(eflags/rflags), XMMX(xmm0, xmm1, xmm2, xmm3, xmm4, xmm5).
	//                               Push the CPU above states to the stack before executing callback.
	//                               You should use this parameter if you perform a mid function hook.
	//                               By default no CPU state is saved.
	template <class _Ty, size_t callbackSize>
	bool HookBuffer(const uintptr_t address, _Ty(&callback)[callbackSize], uintptr_t* const trampoline = nullptr, const DWORD saveCpuStateMask = 0) { return Hook(address, callback, callbackSize, trampoline, saveCpuStateMask); };

	//Hooks an address by passing a buffer as the callback.
	//Parameters:
	//  address          [in] The address to be hooked.
	//  callback         [in] The callback to be executed when the CPU executes 'address'.
	//  callbackSize     [in] The size of the callback in bytes.
	//  trampoline       [out] An optional pointer to a variable that receives the address
	//                        of the trampoline. The trampoline contains the original replaced
	//                        instructions of the 'address' and a jump back to 'address'.
	//  saveCpuStateMask [in] A mask containing a bitwise OR combination of one or more of
	//                        the following macros: GPR(general purpose registers),
	//                        FLAGS(eflags/rflags), XMMX(xmm0, xmm1, xmm2, xmm3, xmm4, xmm5).
	//                        Push the CPU above states to the stack before executing callback.
	//                        You should use this parameter if you perform a mid function hook.
	//                        By default no CPU state is saved.
	bool HookBuffer(const uintptr_t address, const void* const callback, const size_t callbackSize, uintptr_t* const trampoline = nullptr, const DWORD saveCpuStateMask = 0);

	//Removes a previously placed hook at 'address'.
	//Parameters:
	//  address [in] The address to be unhooked.
	bool Unhook(const uintptr_t address);

	//Scans the address space according to 'scanBoundaries' for a nullByte.
	//Parameters:
	//  size           [in]  The size of the code cave.
	//  nullByte       [in]  The byte of the code cave. If -1 is specified,
	//                       the null byte is any byte, that is, FindCodeCave()
	//                       will return any sequence of the same byte.
	//  scanBoundaries [in]  See definition of the ScanBoundaries class.
	//  codeCaveSize   [out] If not NULL, the variable pointed by this argument
	//                       receives the size of the code cave found. If no code
	//                       cave is found, 0(zero) is set.
	//  protection     [in]  Specifies a mask of memory protection constants
	//                       which defines what memory regions will be scanned.
	//                       The default value(-1) specifies that pages with any
	//                       protection between 'start' and 'end' should be scanned.
	//  numThreads     [in]  The number of threads to be used. Thr default argument
	//                       uses the number of CPU cores as the number of threads.
	//  firstMatch     [in]  If true, the address returned(if any) is guaranteed to
	//                       be the first match(i.e. the lowest address on the virtual
	//                       address space that is a match) according to scanBoundaries.
	uintptr_t FindCodeCave(const size_t size, const uint32_t nullByte = 0x00, const ScanBoundaries& scanBoundaries = ScanBoundaries(SCAN_BOUNDARIES::RANGE, 0, -1), size_t* const codeCaveSize = nullptr, const DWORD protection = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY, const size_t numThreads = static_cast<size_t>(std::thread::hardware_concurrency()), const bool firstMatch = false) const;

	//Scans the address space according to 'scanBoundaries' for nullBytes.
	//Parameters:
	//  size           [in]  The size of the code cave.
	//  nullBytes      [in]  The byte of the code cave.
	//  pNullByte      [in]  If a codecave is found and pNullByte is not NULL,
	//                       the byte that the codecave contains is written to
	//                       the variable pointed by pNullByte.
	//  scanBoundaries [in]  See definition of the ScanBoundaries class.
	//  codeCaveSize   [out] If not NULL, the variable pointed by this argument
	//                       receives the size of the code cave found. If no code
	//                       cave is found, 0(zero) is set.
	//  protection     [in]  Specifies a mask of memory protection constants
	//                       which defines what memory regions will be scanned.
	//                       The default value(-1) specifies that pages with any
	//                       protection between 'start' and 'end' should be scanned.
	//  numThreads     [in]  The number of threads to be used. Thr default argument
	//                       uses the number of CPU cores as the number of threads.
	//  firstMatch     [in]  If true, the address returned(if any) is guaranteed to
	//                       be the first match(i.e. the lowest address on the virtual
	//                       address space that is a match) according to scanBoundaries.
	uintptr_t FindCodeCaveBatch(const size_t size, const std::vector<uint8_t>& nullBytes, uint8_t* const pNullByte = nullptr, const ScanBoundaries& scanBoundaries = ScanBoundaries(SCAN_BOUNDARIES::RANGE, 0, -1), size_t* const codeCaveSize = nullptr, const DWORD protection = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY, const size_t numThreads = static_cast<size_t>(std::thread::hardware_concurrency()), const bool firstMatch = false) const;

	//TODO: add string as type.
	//  values         [in/out]	The values. If there're no elements on the set it's 
	//                          considered to be the 'first scan', otherwise it's a
	//                          'next scan'.
	//  scan           [in]     A reference to a Scan struct which specifies how the
	//                          scan should be performed.
	//  alignment      [in]     The address will only be scanned if it's divisible
	//                          by the alignment value.
	//  scanBoundaries [in]     See definition of the ScanBoundaries class.
	//  protection     [in]     Specifies a mask of memory protection constants
	//                          which defines what memory regions will be scanned.
	//                          The default value(-1) specifies that pages with any
	//                          protection between 'start' and 'end' should be scanned.
	//  numThreads     [in]     The number of threads to be used. Thr default argument
	//                          uses the number of CPU cores as the number of threads.
	template<typename T>
	bool ValueScan(std::vector<Value<T>>& values, Scan<T>& scan, const size_t alignment = 4, const ScanBoundaries& scanBoundaries = ScanBoundaries(SCAN_BOUNDARIES::RANGE, 0, -1), const DWORD protect = PAGE_READONLY | PAGE_READWRITE, const size_t numThreads = static_cast<size_t>(std::thread::hardware_concurrency()))
	{
		if (alignment == 0)
			return false;

		uintptr_t start = 0, end = 0;
		if (values.empty())
		{
			switch (scanBoundaries.scanBoundaries)
			{
			case SCAN_BOUNDARIES::RANGE:
				start = scanBoundaries.start, end = scanBoundaries.end;
				break;
			case SCAN_BOUNDARIES::MODULE:
				DWORD moduleSize;
				if (!(start = GetModuleBase(scanBoundaries.moduleName, &moduleSize)))
					return 0;
				end = start + moduleSize;
				break;
			case SCAN_BOUNDARIES::ALL_MODULES:
			{
				struct ValueScanInfo
				{
					std::vector<Value<T>>& values;
					Scan<T>& scan;
					const size_t alignment;
					const DWORD protection;
					const size_t numThreads;
					bool success;
					MemEx* memex;
				};

				ValueScanInfo vsi = { values, scan, alignment, protect, numThreads, true, this };

				EnumModules(GetCurrentProcessId(),
					[](MODULEENTRY32& me, void* param)
					{
						ValueScanInfo* vsi = static_cast<ValueScanInfo*>(param);
						std::vector<Value<T>> values;

						if (!(vsi->success = vsi->memex->ValueScan(values, vsi->scan, vsi->alignment, ScanBoundaries(SCAN_BOUNDARIES::RANGE, reinterpret_cast<uintptr_t>(me.modBaseAddr), reinterpret_cast<uintptr_t>(me.modBaseAddr) + me.modBaseSize), vsi->protection, vsi->numThreads)))
							return false;

						vsi->values.insert(vsi->values.end(), values.begin(), values.end());
						return true;
					}, &vsi);

				return vsi.success;
			}
			default:
				return 0;
			}
		}
		size_t chunkSize = (end - start) / numThreads;
		std::vector<ThreadData<T>> threads(numThreads);

		if (!values.empty())
		{
			size_t quota = values.size() / numThreads;
			for (size_t i = 0; i < threads.size(); i++)
				threads[i].values.insert(threads[i].values.end(), values.begin() + i * quota, values.begin() + (i + 1) * quota);

			values.clear();
		}

		for (size_t i = 0; i < numThreads; i++)
			threads[i].thread = std::thread(&MemEx::ValueScanImpl<T>, this, std::ref(threads[i].values), std::ref(scan), alignment, start + chunkSize * i, start + chunkSize * (static_cast<size_t>(i) + 1), protect);

		for (auto& thread : threads)
			thread.thread.join();

		size_t newCapacity = 0;
		for (auto& thread : threads)
			newCapacity += thread.values.size();

		values.reserve(newCapacity);

		for (auto& thread : threads)
			values.insert(values.end(), thread.values.begin(), thread.values.end());

		return true;
	}

	//Creates and returns a handle to an unnamed file-mapping object backed by the system's 
	//paging system. It basically represents a page which can be shared with other processes.
	//Additionaly, maps a view of the file locally and remotely.
	//Parameters:
	//  size       [in]  The size of the file-mapping object.
	//  localView  [out] A reference to a variable that will receive the locally mapped view.
	//  remoteView [out] A reference to a variable that will receive the remotely mapped view.
	HANDLE AllocateSharedMemory(const size_t size, PVOID& localView, PVOID& remoteView) const;

	//Unmaps the views previously mapped views and deletes the file-mapping object.
	//Parameters:
	//  hFileMapping [in] A handle to a file-mapping object.
	//  localView    [in] The local view.
	//  remoteView   [in] The remote view.
	bool FreeSharedMemory(HANDLE hFileMapping, LPCVOID localView, LPCVOID remoteView) const;

	//Maps a view of a file-mapping object on the address space of the current process.
	//Internally, it's a wrapper around MapViewOfFile().
	//Parameters:
	//  hFileMapping [in] A handle to a file-mapping object created by
	//                    AllocateSharedMemory() or CreateSharedMemory().
	static PVOID MapLocalViewOfFile(const HANDLE hFileMapping);

	//Unmaps a view of a file-mapping object on the address space of the current process.
	//Internally it's a wrapper around UnmapViewOfFile().
	//Parameters:
	//  localAddress [in] The address of the view on the address space of the current process.
	static bool UnmapLocalViewOfFile(LPCVOID localAddress);

	//Maps a view of a file-mapping object on the address space of the opened process.
	//Internally, it's a wrapper around MapViewOfFileNuma2() if available, otherwise
	//perform a workaround.
	//Parameters:
	//  hFileMapping [in] A handle to a file-mapping object created by
	//                    AllocateSharedMemory() or CreateSharedMemory().
	PVOID MapRemoteViewOfFile(const HANDLE hFileMapping) const;

	//Unmaps a view of a file-mapping object on the address space of the opened process.
	//Internally it's a wrapper around UnmapViewOfFile2() if available, otherwise
	//perform a workaround.
	//Parameters:
	//  localAddress [in] The address of the view on the address space of the opened process.
	bool UnmapRemoteViewOfFile(LPCVOID remoteAddress) const;

	//Returns the PID of the specified process.
	//Parameters:
	//  processName [in] The name of the process.
	static DWORD GetProcessIdByName(const TCHAR* const processName);

	//Returns the PID of the window's owner.
	//Parameters:
	//  windowName [in] The window's title. If NULL, all window 
	//                  names match.
	//  className  [in] The class name. If NULL, any window title
	//                  matching windowName is considered.
	static DWORD GetProcessIdByWindow(const TCHAR* const windowName, const TCHAR* const className = nullptr);

	//If moduleName is NULL, GetModuleBase() returns the base of the module created by the file used to create the process specified (.exe file)
	//Returns a module's base address on the opened process.
	//Parameters:
	//  moduleName  [in]  The name of the module.
	//  pModuleSize [out] An optional pointer that if provided, receives the size of the module.
	uintptr_t GetModuleBase(const TCHAR* const moduleName = nullptr, DWORD* const pModuleSize = nullptr) const;

	//Returns a module's base address on the process specified by dwProcessId.
	//Parameters:
	//  dwProcessId [in]  The PID of the process where the module base is retried.
	//  moduleName  [in]  The name of the module.
	//  pModuleSize [out] An optional pointer that if provided, receives the size of the module.
	static uintptr_t GetModuleBase(const DWORD dwProcessId, const TCHAR* const moduleName = nullptr, DWORD* const pModuleSize = nullptr);

	//Returns the size of first parsed instruction on the buffer at 'address'.
	//Parameters:
	//  address [in] The address of the buffer containing instruction.
	size_t GetInstructionLength(const uintptr_t address);

	//Loops through all modules of a process passing its information to a callback function.
	//Parameters:
	//  processId [in] The PID of the process which the modules will be looped.
	//  callback  [in] A function pointer to a callback function.
	//  param     [in] An optional pointer to be passed to the callback.
	static void EnumModules(const DWORD processId, bool (*callback)(MODULEENTRY32& me, void* param), void* param);

	//Converts an AOB in string form into pattern & mask form.
	//Parameters:
	//  AOB     [in]  The array of bytes(AOB) in string form.
	//  pattern [out] The string that will receive the pattern.
	//  mask    [out] The string that will receive th mask.
	static void AOBToPattern(const char* const AOB, std::string& pattern, std::string& mask);

	//Converts a pattern and mask into an AOB.
	//Parameters:
	//  pattern [in]  The pattern.
	//  mask    [in]  The mask.
	//  AOB     [out] The array of bytes(AOB) in string form.
	static void PatternToAOB(const char* const pattern, const char* const mask, std::string& AOB);

	//Returns the size of a page on the system.
	static DWORD GetPageSize();

	//Creates and returns a handle to an unnamed file-mapping object backed by the system's 
	//paging system. It basically represents a page which can be shared with other processes.
	//Parameters:
	//  size [in] The size of the file-mapping object.
	static HANDLE CreateSharedMemory(const size_t size);

	//Injects a dll into the opened process. If you choose to use
	//manual mapping, it's recommended to compile in release mode.
	//The function fails if 'injectionMethod' is LOAD_LIBRARY and
	//'isPath' is false. The base of the injected module is returned
	//Parameters:
	//  dll             [in] See the 'isPath' parameter.
	//  injectionMethod [in] The injection method.
	//  isPath          [in] If true, 'dll' specifies the path to the dll,
	//otherwise 'dll' is a pointer to the dll in memory.
	uintptr_t Inject(const void* dll, INJECTION_METHOD injectionMethod = INJECTION_METHOD::LOAD_LIBRARY, bool isPath = true);

	//Retrieves the address of a function from the opened process.
	//  moduleBase    [in]  The module's base on the opened process.
	//  procedureName [in]  The procedure's name on the opened process.
	//  pOrdinal      [out] A pointer to a variable that receives the procedure's ordinal.
	uintptr_t GetProcAddressEx(uintptr_t moduleBase, const char* procedureName, uint16_t* const pOrdinal = nullptr);

private:
	void PatternScanImpl(std::atomic<uintptr_t>& address, const uint8_t* const pattern, const char* const mask, uintptr_t start, const uintptr_t end, const DWORD protect, const bool firstMatch) const;

	void* CallImpl(const CConv cConv, const bool isReturnFloat, const bool isReturnDouble, const size_t returnSize, const uintptr_t functionAddress, std::vector<Arg>& args);

	void FindCodeCaveImpl(std::atomic<uintptr_t>& returnValue, const size_t size, uintptr_t start, const uintptr_t end, const DWORD protect, const bool firstMatch) const;

	template<typename T> Arg GetArgument(T& t) { return Arg(&t, sizeof(t), true, true); }
	Arg GetArgument(const char t[]) { return Arg(t, strlen(t) + 1, true, false, true); }
	Arg GetArgument(const wchar_t t[]) { return Arg(t, (static_cast<size_t>(lstrlenW(t)) + 1) * 2, true, false, true); }
	Arg GetArgument(Arg& t) { return t; }

#ifdef _WIN64
	Arg GetArgument(float& t) { Arg arg(&t, sizeof(float), true, true); arg.isFloat = true; return arg; }
	Arg GetArgument(double& t) { Arg arg(&t, sizeof(double), true, true); arg.isFloat = true; return arg; }
#endif

	void GetArguments(std::vector<Arg>& args) {}

	template<typename T, typename ... Args>
	void GetArguments(std::vector<Arg>& args, T& first, Args&& ... arguments)
	{
		args.emplace_back(GetArgument(first));

		GetArguments(args, arguments...);
	}

	bool SetupRemoteThread();
	void DeleteRemoteThread();

	template<typename T>
	struct ValueScanRegionData
	{
		std::vector<Value<T>>& values;
		Scan<T>& scan;
		size_t alignment;
		uintptr_t start, end;
		Value<T>* value;
		uintptr_t localBuffer;
		uintptr_t targetBuffer;

		ValueScanRegionData(std::vector<Value<T>>& values, Scan<T>& scan, size_t alignment, uintptr_t start, uintptr_t end, uintptr_t localBuffer, Value<T>* value = nullptr, uintptr_t targetBuffer = NULL)
			: values(values),
			scan(scan),
			alignment(alignment),
			start(start),
			end(end),
			value(value),
			localBuffer(localBuffer),
			targetBuffer(targetBuffer) {}
	};

	template<typename T>
	static T ProcessValue(ValueScanRegionData<T>& vsrd) { return *reinterpret_cast<const T*>(vsrd.start); }

	template<>
	static float ProcessValue(ValueScanRegionData<float>& vsrd)
	{
		if (vsrd.scan.floatRounding == FLOAT_ROUNDING::ROUND)
			return roundf(*reinterpret_cast<float*>(vsrd.start));
		else if (vsrd.scan.floatRounding == FLOAT_ROUNDING::TRUNCATE)
			return truncf(*reinterpret_cast<float*>(vsrd.start));
		else
			return *reinterpret_cast<float*>(vsrd.start);
	}

	template<>
	static double ProcessValue(ValueScanRegionData<double>& vsrd)
	{
		if (vsrd.scan.floatRounding == FLOAT_ROUNDING::ROUND)
			return round(*reinterpret_cast<double*>(vsrd.start));
		else if (vsrd.scan.floatRounding == FLOAT_ROUNDING::TRUNCATE)
			return trunc(*reinterpret_cast<double*>(vsrd.start));
		else
			return *reinterpret_cast<double*>(vsrd.start);
	}

	template<typename T>
	static void ValueScanRegionEquals(ValueScanRegionData<T>& vsrd)
	{
		for (; vsrd.start < vsrd.end; vsrd.start += vsrd.alignment)
		{
			if (ProcessValue(vsrd) == vsrd.scan.value)
				vsrd.values.emplace_back(vsrd.targetBuffer + (vsrd.start - vsrd.localBuffer), *reinterpret_cast<T*>(vsrd.start));
		}
	}

	template<typename T>
	static void ValueScanRegionGreater(ValueScanRegionData<T>& vsrd)
	{
		for (; vsrd.start < vsrd.end; vsrd.start += vsrd.alignment)
		{
			if (ProcessValue(vsrd) > vsrd.scan.value)
				vsrd.values.emplace_back(vsrd.targetBuffer + (vsrd.start - vsrd.localBuffer), *reinterpret_cast<T*>(vsrd.start));
		}
	}

	template<typename T>
	static void ValueScanRegionLess(ValueScanRegionData<T>& vsrd)
	{
		for (; vsrd.start < vsrd.end; vsrd.start += vsrd.alignment)
		{
			if (ProcessValue(vsrd) < vsrd.scan.value)
				vsrd.values.emplace_back(vsrd.targetBuffer + (vsrd.start - vsrd.localBuffer), *reinterpret_cast<T*>(vsrd.start));
		}
	}

	template<typename T>
	static void ValueScanRegionBetween(ValueScanRegionData<T>& vsrd)
	{
		for (; vsrd.start < vsrd.end; vsrd.start += vsrd.alignment)
		{
			if (ProcessValue(vsrd) > vsrd.scan.value&&* reinterpret_cast<T*>(vsrd.start) < vsrd.scan.value2)
				vsrd.values.emplace_back(vsrd.targetBuffer + (vsrd.start - vsrd.localBuffer), *reinterpret_cast<T*>(vsrd.start));
		}
	}

	template<typename T>
	static void ValueScanRegionUnknown(ValueScanRegionData<T>& vsrd)
	{
		for (; vsrd.start < vsrd.end; vsrd.start += vsrd.alignment)
			vsrd.values.emplace_back(vsrd.targetBuffer + (vsrd.start - vsrd.localBuffer), *reinterpret_cast<T*>(vsrd.start));
	}

	template<typename T>
	static T NextScanProcessValue(ValueScanRegionData<T>& vsrd) { return *reinterpret_cast<const T*>(vsrd.localBuffer); }

	template<>
	static float NextScanProcessValue(ValueScanRegionData<float>& vsrd)
	{
		if (vsrd.scan.floatRounding == FLOAT_ROUNDING::ROUND)
			return roundf(*reinterpret_cast<float*>(vsrd.localBuffer));
		else if (vsrd.scan.floatRounding == FLOAT_ROUNDING::TRUNCATE)
			return truncf(*reinterpret_cast<float*>(vsrd.localBuffer));
		else
			return *reinterpret_cast<float*>(vsrd.localBuffer);
	}

	template<>
	static double NextScanProcessValue(ValueScanRegionData<double>& vsrd)
	{
		if (vsrd.scan.floatRounding == FLOAT_ROUNDING::ROUND)
			return round(*reinterpret_cast<double*>(vsrd.localBuffer));
		else if (vsrd.scan.floatRounding == FLOAT_ROUNDING::TRUNCATE)
			return trunc(*reinterpret_cast<double*>(vsrd.localBuffer));
		else
			return *reinterpret_cast<double*>(vsrd.localBuffer);
	}

	template<typename T>
	static bool NextValueScanEquals(ValueScanRegionData<T>& vsrd)
	{
		return NextScanProcessValue(vsrd) == vsrd.scan.value;
	}

	template<typename T>
	static bool NextValueScanGreater(ValueScanRegionData<T>& vsrd)
	{
		return NextScanProcessValue(vsrd) > vsrd.scan.value;
	}

	template<typename T>
	static bool NextValueScanLess(ValueScanRegionData<T>& vsrd)
	{
		return NextScanProcessValue(vsrd) < vsrd.scan.value;
	}

	template<typename T>
	static bool NextValueScanBetween(ValueScanRegionData<T>& vsrd)
	{
		T value = NextScanProcessValue(vsrd);
		return value > vsrd.scan.value&& value < vsrd.scan.value2;
	}

	template<typename T>
	static bool NextValueScanIncreased(ValueScanRegionData<T>& vsrd)
	{
		return NextScanProcessValue(vsrd) > vsrd.value->value;
	}

	template<typename T>
	static bool NextValueScanIncreasedBy(ValueScanRegionData<T>& vsrd)
	{
		T value = NextScanProcessValue(vsrd);
		return value == value + vsrd.scan.value;
	}

	template<typename T>
	static bool NextValueScanDecreased(ValueScanRegionData<T>& vsrd)
	{
		return NextScanProcessValue(vsrd) < vsrd.value->value;
	}

	template<typename T>
	static bool NextValueScanDecreasedBy(ValueScanRegionData<T>& vsrd)
	{
		T value = NextScanProcessValue(vsrd);
		return value == value - vsrd.scan.value;
	}

	template<typename T>
	static bool NextValueScanChanged(ValueScanRegionData<T>& vsrd)
	{
		return NextScanProcessValue(vsrd) != vsrd.value->value;
	}

	template<typename T>
	static bool NextValueScanUnchanged(ValueScanRegionData<T>& vsrd)
	{
		return NextScanProcessValue(vsrd) == vsrd.value->value;
	}

	template<typename T>
	static void PerformFloatRounding(Scan<T>& scan) {}

	template<>
	static void PerformFloatRounding(Scan<float>& scan)
	{
		if (scan.floatRounding == FLOAT_ROUNDING::ROUND)
			scan.value = roundf(scan.value), scan.value2 = roundf(scan.value2);
		else if (scan.floatRounding == FLOAT_ROUNDING::TRUNCATE)
			scan.value = truncf(scan.value), scan.value2 = truncf(scan.value2);
	}

	template<>
	static void PerformFloatRounding(Scan<double>& scan)
	{
		if (scan.floatRounding == FLOAT_ROUNDING::ROUND)
			scan.value = round(scan.value), scan.value2 = round(scan.value2);
		else if (scan.floatRounding == FLOAT_ROUNDING::TRUNCATE)
			scan.value = trunc(scan.value), scan.value2 = trunc(scan.value2);
	}

	template<typename T>
	void ValueScanImpl(std::vector<Value<T>>& values, Scan<T>& scan, const size_t alignment, uintptr_t start, const uintptr_t end, const DWORD protect)
	{
		PerformFloatRounding<T>(scan);
		char buffer[4096];
		SIZE_T nBytesRead;

		ValueScanRegionData<T> vsrd(values, scan, alignment, 0, end, reinterpret_cast<uintptr_t>(buffer));
		MEMORY_BASIC_INFORMATION mbi;
		if (values.empty())
		{
			void(*firstScan)(ValueScanRegionData<T>&);
			switch (scan.scanType)
			{
			case SCAN_TYPE::EXACT: firstScan = ValueScanRegionEquals; break;
			case SCAN_TYPE::GREATER_THAN: firstScan = ValueScanRegionGreater; break;
			case SCAN_TYPE::LESS_THAN: firstScan = ValueScanRegionLess; break;
			case SCAN_TYPE::BETWEEN: firstScan = ValueScanRegionBetween; break;
			case SCAN_TYPE::UNKNOWN: firstScan = ValueScanRegionUnknown; break;
			default: return;
			}

			while (start < end && VirtualQueryEx(m_hProcess, reinterpret_cast<LPCVOID>(start), &mbi, sizeof(MEMORY_BASIC_INFORMATION)))
			{
				if (mbi.Protect & protect)
				{
					const uintptr_t regionEnd = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
					for (; start < regionEnd; start += 4096)
					{
						vsrd.targetBuffer = start;
						vsrd.start = reinterpret_cast<uintptr_t>(buffer);

						if (!ReadProcessMemory(m_hProcess, reinterpret_cast<LPCVOID>(start), buffer, 4096, &nBytesRead))
							break;

						vsrd.end = reinterpret_cast<uintptr_t>(buffer) + nBytesRead;

						if (((vsrd.end - vsrd.start) % sizeof(T)) > 0)
							vsrd.end -= ((vsrd.end - vsrd.start) % sizeof(T));

						firstScan(vsrd);
					}
				}

				start = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
			}
		}
		else
		{
			bool(*nextScan)(ValueScanRegionData<T>&);
			switch (scan.scanType)
			{
			case SCAN_TYPE::EXACT: nextScan = NextValueScanEquals; break;
			case SCAN_TYPE::GREATER_THAN: nextScan = NextValueScanGreater; break;
			case SCAN_TYPE::LESS_THAN: nextScan = NextValueScanLess; break;
			case SCAN_TYPE::BETWEEN: nextScan = NextValueScanBetween; break;
			case SCAN_TYPE::INCREASED: nextScan = NextValueScanIncreased; break;
			case SCAN_TYPE::INCREASED_BY: nextScan = NextValueScanIncreasedBy; break;
			case SCAN_TYPE::DECREASED: nextScan = NextValueScanDecreased; break;
			case SCAN_TYPE::DECREASED_BY: nextScan = NextValueScanDecreasedBy; break;
			case SCAN_TYPE::CHANGED: nextScan = NextValueScanChanged; break;
			case SCAN_TYPE::UNCHANGED: nextScan = NextValueScanUnchanged; break;
			default: return;
			}

			std::sort(values.begin(), values.end());
			vsrd.start = reinterpret_cast<uintptr_t>(buffer);

			for (size_t i = 0; i < values.size();)
			{
				if (values[i].address > vsrd.end)
				{
					VirtualQueryEx(m_hProcess, reinterpret_cast<LPCVOID>(values[i].address), &mbi, sizeof(MEMORY_BASIC_INFORMATION));
					ReadProcessMemory(m_hProcess, reinterpret_cast<LPCVOID>(values[i].address), buffer, static_cast<SIZE_T>(reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize - values[i].address), &nBytesRead);
					vsrd.start = values[i].address, vsrd.end = values[i].address + nBytesRead;
				}

				vsrd.localBuffer = reinterpret_cast<uintptr_t>(buffer) + values[i].address - vsrd.start;
				vsrd.value = &values[i];
				if (nextScan(vsrd))
				{
					i++;
					continue;
				}

				values.erase(values.begin() + i);
			}
		}
	}

	template<>
	void ValueScanImpl(std::vector<Value<AOB>>& values, Scan<AOB>& scan, const size_t alignment, uintptr_t start, const uintptr_t end, const DWORD protect)
	{
		if (values.empty())
		{
			uintptr_t aobStart = start;
			size_t patternSize;

			while ((aobStart = AOBScan(scan.value.aob, ScanBoundaries(SCAN_BOUNDARIES::RANGE, aobStart, end), protect, &patternSize, 1)))
			{
				if (std::find_if(std::begin(values), std::end(values), [aobStart](Value<AOB>& value) { return aobStart == value.address; }) == std::end(values))
				{
					AOB aob(reinterpret_cast<const char*>(aobStart), patternSize);
					Value<AOB> value(aobStart++, aob);
					values.push_back(value);
				}
			}
		}
		else
		{
			std::sort(values.begin(), values.end());

			uintptr_t regionEnd = 0;

			MEMORY_BASIC_INFORMATION mbi;
			for (size_t i = 0; i < values.size();)
			{
				if (values[i].address < regionEnd || VirtualQueryEx(m_hProcess, reinterpret_cast<void*>(values[i].address), &mbi, sizeof(MEMORY_BASIC_INFORMATION)) && !(mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) && (regionEnd = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize))
				{
					bool unchanged = true;
					for (size_t j = 0; j < values[i].value.size; j++)
					{
						if (reinterpret_cast<uint8_t*>(values[i].address)[j] != values[i].value.aob[j])
						{
							unchanged = false;
							break;
						}
					}

					if (unchanged)
					{
						i++;
						continue;
					}
				}

				values.erase(values.begin() + i);
			}
		}
	}
};

#endif // MEMEX_H