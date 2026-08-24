#include <iostream>
#include <fstream>
#include <streambuf>

//We initialize the class connected to the streambuf to gain access to low-level C++ functions, allowing us to duplicate the buffer that is sent to the terminal.
class log : public std::streambuf {
    public:
    log(std::streambuf* terminal, std::streambuf* logs) : terminal(terminal), logs(logs) {}
    //We use the constructor to initialize the class with objects or `streambuf` parameters. Note that we use pointers to access their memory addresses: the first address corresponds to the terminal output (what appears on the screen), while the second corresponds to the destination for the ".log" file used by our program.
    protected:
    virtual int overflow(int c){
        if (c == EOF) return EOF; //If is end or is one char return EOF
        if ((terminal->sputc(c) == EOF) || (logs->sputc(c) == EOF)) return EOF; //Similar for terminal and log
        return c; //Returning C if every is okey
    }
    virtual int sync(){
        return terminal->pubsync() == 0 && logs->pubsync() == 0 ? 0 : -1; //Calling the `sync` function—another function within `streambuf`—helps avoid the tedium of writing from RAM to the terminal; `sync` (via `pubsync`) empties the RAM buffer and sends the data directly to the destination—in this case, the terminal and the .log file.
    }
    private:
    std::streambuf* terminal; //Parameters.
    std::streambuf* logs;
};

void Function(){
    std::cout<< "Ejecuting.."<<std::endl; //Function inicializing in the moment of ejecuting.
}

int main(){
    std::ofstream logFile(".log", std::ios::app);
    //We open the file for writing in this, but note that we use `std::ios::app` so that it doesn't erase previously saved data, but instead continues writing after it, saving everything in the `.log` file.
    log tee(std::cout.rdbuf(), logFile.rdbuf());
    //Creating the class and preparing all.
    std::streambuf* old_dir = std::cout.rdbuf(&tee);
    //Start the magic, std::cout.rdbuf(&tee), Initialize everything using `tee` to capture the data sent by the client from `std::cout`, and use `rdbuf()` to connect everything to the `streambuf`. Since the original stream must still be passed—due to the functions called within the class—it also returns the original value, the "last" one.

    std::cout<<"---Inicializing Program---"<<std::endl; //Initialize
    Function(); //Ejecuting.
    std::cout<<"---End of Program---"<<std::endl;//Ending note.
    std::cout.rdbuf(old_dir); //returning the original cout.
    return 0;
}
//Note how we had to replicate certain functions that operate beneath `std::cout` to make everything work in C++. These are low-level language functions used to display text on the screen;
//we simply call them and utilize specific features—such as `overflow` or `async` combined with `virtual`—to duplicate the text and send it to the `.log` file, all using native C++ mechanisms.
