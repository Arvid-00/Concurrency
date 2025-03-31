#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <type_traits>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>

std::vector<int> myVector(20, 0); //changed to global, because you can't pass refereances with thread
const int range = 4;

std::mutex mutex;
std::condition_variable dataCV;

std::vector<std::string> text;
int totalMatches = 0;
int totalExecitionTime = 0;

//template<class T>
void IncrementVector(int idvalue)
{
	for (int i = idvalue * 4; i < idvalue * 4 + 4; i++)
		myVector[i] = idvalue;
}
void Test1()
{
	const int maxThreads = 5;
	std::thread threads[maxThreads];

	std::cout << "<Increment function will now be called>" << std::endl;
	for (int i = 0; i < maxThreads; i++)
		threads[i] = std::thread(IncrementVector, i);

	for (int i = 0; i < maxThreads; i++)
		threads[i].join();

	std::cout << "Vector now contians: " << std::endl;
	for (int i = 0; i < myVector.size(); i++)
		std::cout << myVector[i];
	std::cout << std::endl;

	std::cout << "Increment test is complete" << std::endl;
}

void Test2Function(int id) 
{
	std::lock_guard<std::mutex> lock(mutex);
	std::cout << "Hello" << " from " << " thread " << id << "!" << std::endl;
}

void Test2() 
{
	const int nbrOfT = 10;
	std::thread t[nbrOfT];
	for (int i = 0; i < nbrOfT; i++) 
	{
		t[i] = std::thread(Test2Function, i);
	}
	for (int i = 0; i < nbrOfT; i++) 
	{
		t[i].join();
	}

}

void ParseText(int id, int lines, std::string word)
{
	
	auto start = std::chrono::high_resolution_clock::now();

	int occurances = 0;
	for (int j = lines * id; j < lines * id + lines; j++) 
	{
		if (text[j].find(word) != std::string::npos)
		{
			occurances++;
			totalMatches++;
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	auto executionTime = duration.count();

	mutex.lock();
	totalExecitionTime += executionTime;
	std::cout << "Thread ID: " << std::this_thread::get_id() << " found " << occurances 
		<< " occurances of '" << word << "' in " << executionTime << "ms" << std::endl;
	mutex.unlock();

}

void Test3() 
{
	std::string filename = "bible.txt";
	std::ifstream file;
	file.open(filename);
	std::string line; 
	while (!file.eof())
	{
		std::getline(file, line);
		if(line != "") //removing emtpy lines
			text.push_back(line);
	}

	const int nbrOfT = 4;
	const std::string wordToFind = "apple";
	const int linesToRead = text.size() / nbrOfT;
	std::cout << "Available hardware threads: " << std::thread::hardware_concurrency() << std::endl;
	std::cout << "Amount of threads using: " << nbrOfT << std::endl;
	std::cout << "Lines per thread: " << linesToRead << std::endl;
	std::cout << "Searching '" << filename << "' for instances of: '" << wordToFind << "'" << std::endl;
	std::thread t[nbrOfT];
	
	for (int i = 0; i < nbrOfT; i++)
		t[i] = std::thread(ParseText, i, (text.size() / nbrOfT), wordToFind);
	for (int i = 0; i < nbrOfT; i++)
		t[i].join();

	std::cout << "Total matches: " << totalMatches << std::endl;
	std::cout << "Total execution time: " << totalExecitionTime << " ms" << std::endl;
}

class Item
{
public:
	int id;
	int timer;
	Item(int _id, int _timer) 
	{
		id = _id;
		timer = _timer;
	}
	Item() = default;

};

std::vector<Item> orderList;
const int maxitems = 5;
const int nbrOfCT = 3;
const int nbrOfPT = 9;
int itemsProcessed = 0;

void Consume(int id) 
{
	int progress = 0;
	Item item;
	for (int i = 0; i < (nbrOfPT / nbrOfCT); i++)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		std::unique_lock lock{ mutex };
		{
			dataCV.wait(lock, [&]() { return orderList.size() > 0; });
			
			item = orderList.back();
			orderList.pop_back();
			lock.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(item.timer / 5));
			lock.lock();
			std::cout << "Barista " << id << " preparing order: " << item.id << std::endl;

			while (progress < 100) 
			{
				lock.unlock();
				std::this_thread::sleep_for(std::chrono::milliseconds(item.timer / 5));
				lock.lock();
				progress += 20;
				std::cout << "Barista " << id << " progress: " << progress << "%" << std::endl;
			}

			lock.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(item.timer / 5));
			lock.lock();
			std::cout << "Barista " << id << " served order: " << item.id << std::endl;
			progress = 0;
		}
		dataCV.notify_one();
	}

}

void Produce(int id) 
{
	
	std::unique_lock lock{ mutex };
	dataCV.wait(lock, [=]() { return orderList.size() < maxitems; });
	
	std::cout << "Customer " << id << " placed order: " << id << std::endl;
	std::this_thread::sleep_for(std::chrono::milliseconds(300));
	Item item(id, ((id + 1) * 500));
	orderList.push_back(item);

	dataCV.notify_one();
}

void Test4() 
{

	std::thread ct[nbrOfCT];
	std::thread pt[nbrOfPT];

	for (int i = 0; i < nbrOfCT; i++)
		ct[i] = std::thread(Consume, i);
	for (int i = 0; i < nbrOfPT; i++)
		pt[i] = std::thread(Produce, i);


	for (int i = 0; i < nbrOfCT; i++)
	{
		ct[i].join();
	}
	for (int i = 0; i < nbrOfPT; i++)
	{
		pt[i].join();
	}
		

	std::cout << "Complete" << std::endl;
	
}

int main() 
{
	std::cout << "_____TEST 1_____" << std::endl;
	Test1();
	std::cout << std::endl;

	std::cout << "_____TEST 2_____" << std::endl;
	Test2();
	std::cout << std::endl;

	std::cout << "_____TEST 3_____" << std::endl;
	Test3();
	std::cout << std::endl;

	std::cout << "_____TEST 4_____" << std::endl;
	Test4();
	std::cout << std::endl;


}

