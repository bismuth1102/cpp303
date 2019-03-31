## c++ project of a concurrent database.

### p1
Server generates rsa keys(public and private). There are two kinds of format of message that the clients transmit. First is for KEY and BYE: KEY/n or BYE/n. Second is four lines as below, all of the lines are concatenate with `/n`:
1. The encrypted aes key (by public key)
2. The encrypted command (by aes bey)
3. The encrypted number of bytes of the fourth line (by aes key)
4. The infomation that server needs to know for this command. All of the commands which use this format need client's username and password.

When server trying to decrypt the message, it should first use its private key to decrypt aes key, and then use the aes key to decrypt other messages.<br>
At last, server will persist the data in a file.<br>
One of the difficulty is malloc and free. When you want to initialize an unsigned char* with `i` bytes, first malloc `i+1` bytes, then memset these `i+1` bytes with `0`, finally use memcpy. When using `eof` to decide weather the socket gets to the end or not, the extra `0` when we malloc and memset is the symbol for eof.

### p2
We focus on thread pool and concurrent database in this project. In project 1, we use `std::map` to store data. However, `std::map` does not have fine-grained concurrency. Instead, we use a vector of classes. Each class include a std::mutex and a vector of Entries. Entry is a collection of data.<br>
Since the operations on database is concurrent, when we use command `ALL`(get all usernames) and `SAV`(persist), we need two-phase locking in order to avoid other threads change any data while travesing and persisting. We also avoid dead locks because each two-phase locking is from the first class in the vector to the last one, no circle will generate.

### p3
In project 2, since `SAV` needs two-phase locking, we cannot persist after each operation. This will cause a problem that if we have already changed the data a lot but not `SAV` yet, the computer crashes, all the data changes disappear. So we record after each operation. If computer crashes before `SAV`, server can generate the data structure based on the log.


