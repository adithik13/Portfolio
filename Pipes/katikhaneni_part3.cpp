#include <iostream>
#include <algorithm>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

void parent(int pipe_1[], int pipe_2[], int pipe_4[], std::vector<int> nums) {
	close(pipe_1[0]);
	close(pipe_2[1]);
	close(pipe_4[1]);

	write(pipe_1[1], nums.data(), nums.size() * sizeof(int));
	close(pipe_1[1]);

	std::vector<int> nums_sorted(nums.size());
	read(pipe_2[0], nums_sorted.data(), nums_sorted.size() * sizeof(int));
	close(pipe_2[0]);

	//std::cout << "nums sorted: ";
	for (int num : nums_sorted) {
		std::cout << num << "  ";
	}
	std::cout << std::endl;

	int median;
	read(pipe_4[0], &median, sizeof(median));
	close(pipe_4[0]);

	std::cout << "Median: " << median << std::endl;
}

void child_1(int pipe_1[], int pipe_2[], int pipe_3[]) {
	close(pipe_1[1]);
	close(pipe_2[0]);
	close(pipe_3[0]);

	std::vector<int> nums(5);
	read(pipe_1[0], nums.data(), nums.size() * sizeof(int));
	close(pipe_1[0]);

	std::sort(nums.begin(), nums.end());

	write(pipe_2[1], nums.data(), nums.size() * sizeof(int));
	write(pipe_3[1], nums.data(), nums.size() * sizeof(int));
	close(pipe_2[1]);
	close(pipe_3[1]);

	exit(0);
}

void child_2(int pipe_3[], int pipe_4[]) {
	close(pipe_3[1]);
	close(pipe_4[0]);

	std::vector<int> nums_sorted(5);
	read(pipe_3[0], nums_sorted.data(), nums_sorted.size() * sizeof(int));
	close(pipe_3[0]);

	int median = nums_sorted[nums_sorted.size() / 2];

	write(pipe_4[1], &median, sizeof(median));
	close(pipe_4[1]);

	exit(0);
}

int main(int argc, char* argv[]) {

	std::vector<int> nums;
	for (int i = 1; i < argc; ++i) {
		nums.push_back(std::stoi(argv[i]));
	}

	int pipe_1[2], pipe_2[2], pipe_3[2], pipe_4[2];
	pipe(pipe_1);
	pipe(pipe_2);
	pipe(pipe_3);
	pipe(pipe_4);

	if (fork() == 0) {
		child_1(pipe_1, pipe_2, pipe_3);
	}

	if (fork() == 0) {
		child_2(pipe_3, pipe_4);
	}

	parent(pipe_1, pipe_2, pipe_4, nums);

	wait(0);
	wait(0);

	return 0;
}
