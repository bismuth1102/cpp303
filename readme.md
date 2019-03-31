**c++ project of a concurrent database.**

**p1**<br>
Server generates rsa keys(public and private). There are two kinds of format that the clients transmit. First is for KEY and BYE: KEY/n or BYE/n. Second format is four lines as below, all of the lines are concatenate with "/n":
1. The encrypted aes key (by public key)
2. The encrypted command (by aes bey)
3. The encrypted number of bytes of the fourth line (by aes key)
4. The infomation that server needs to know for this command. All of the commands which use this format need client's username and password.

When server trying to decrypt the message, it should first use its private key to decrypt aes key, and then use the aes key to decrypt other messages.<br>
At last, server will persist the data in a file.<br>

**p2**<br>



