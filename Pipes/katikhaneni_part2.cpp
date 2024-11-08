#include <iostream>
#include <fstream>
#include <string>

int main(){
	std::ifstream ex7("ex7_Pipe");
	if(!ex7.is_open()){
		std::cerr<< "cannot open pipe" << std::endl;
		return 1;
	}
	std::string input;
	int operation = 0;

	while(std::getline(ex7, input)){
		operation++;
		if(input.find("failed") != std::string::npos){
			std::cout<<"Program failed on operation " << operation << std::endl;
			return 0;
		}
	}

	std::cout<<" all good" << std::endl;
	return 0;
}
