#include <windows.h>

class SyncQueue {
private:
	int* array;
	int size;
	int insertPosition;
	int removePosition;
	HANDLE availableToDelete;
	HANDLE availableToAdd;
	CRITICAL_SECTION criticalSection;
public:
	SyncQueue(int size);

	~SyncQueue();

	void Insert(int element);

	int Remove();
};