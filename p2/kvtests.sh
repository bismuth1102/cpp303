# [303] This is not a comprehensive test suite, but it helps to test the
#       correctness of the key/value implementation.

# a few file names that get repeated a lot
file1=Makefile
file2=protocol.h
file3=server_storage.h
file4=obj64/common_net.o

# number of threads
numthreads=4

# Reset file system from any prior failures
rm -f rsa.pri rsa.pub company.dir # server makes these
rm -f localhost.pub               # client makes these

echo ""

echo -n "Starting server. Expect 'File not found: company.dir'      "
./obj64/server.exe -t $numthreads -p 9999 -k rsa -f company.dir > /dev/null &
sleep 1

echo -n "Registering new user alice. Expect OK                      "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -R
echo -n "Getting nonexistent key.  Expect ERR_BADKEY                "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -T makefile -v getkey.file
echo -n "Getting for nonexistent user.  Expect ERR_USER_EXISTS      "
obj64/client.exe -k localhost.pub -u bob -w alice_is_awesome -s localhost -p 9999 -T makefile -v getkey.file
echo -n "Getting with bad password.  Expect ERR_LOGIN               "
obj64/client.exe -k localhost.pub -u alice -w bob_is_awesome -s localhost -p 9999 -T makefile -v getkey.file

echo -n "Setting key.  Expect OK                                    "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -I makefile -v $file1
echo -n "Setting repeat key.  Expect ERR_BADKEY                     "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -I makefile -v $file1
echo -n "Setting with nonexistent user.  Expect ERR_USER_EXISTS     "
obj64/client.exe -k localhost.pub -u bob -w alice_is_awesome -s localhost -p 9999 -I makefile -v $file1
echo -n "Setting with bad password.  Expect ERR_LOGIN               "
obj64/client.exe -k localhost.pub -u alice -w bob_is_awesome -s localhost -p 9999 -I makefile -v $file1
echo -n "Getting key/value.  Expect OK                              "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -T makefile -v getkey.file
diff $file1 getkey.file
rm -f getkey.file

echo -n "Updating key.  Expect OK                                   "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -U makefile -v $file2
echo -n "Getting new key/value.  Expect OK                          "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -T makefile -v getkey.file
diff $file2 getkey.file
rm -f getkey.file
echo -n "Updating nonexistent key.  Expect ERR_BADKEY               "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -U makefile2 -v $file2
echo -n "Updating with nonexistent user.  Expect ERR_USER_EXISTS    "
obj64/client.exe -k localhost.pub -u bob -w alice_is_awesome -s localhost -p 9999 -U makefile -v $file1
echo -n "Updating with bad password.  Expect ERR_LOGIN              "
obj64/client.exe -k localhost.pub -u alice -w bob_is_awesome -s localhost -p 9999 -U makefile -v $file1

echo -n "Deleting key.  Expect OK                                   "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -D makefile
echo -n "Deleting nonexistent key.  Expect ERR_BADKEY               "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -D makefile
echo -n "Deleting with nonexistent user.  Expect ERR_USER_EXISTS    "
obj64/client.exe -k localhost.pub -u bob -w alice_is_awesome -s localhost -p 9999 -D makefile
echo -n "Deleting with bad password.  Expect ERR_LOGIN              "
obj64/client.exe -k localhost.pub -u alice -w bob_is_awesome -s localhost -p 9999 -D makefile

echo -n "Setting key.  Expect OK                                    "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -I makefile -v $file1
echo -n "Instructing server to persist data.  Expect OK             "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -V

echo "----------------------------------------------------------------"

echo "Stopping server.  Expect Server terminated and OK             "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -B

echo ""

echo "Re-starting server to check that data persisted correctly"
./obj64/server.exe -t $numthreads -p 9999 -k rsa -f company.dir > /dev/null &
sleep 1

echo -n "Getting key/value.  Expect OK                              "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -T makefile -v getkey.file
diff $file1 getkey.file
rm -f getkey.file

echo -n "Registering user bob.  Expect OK                           "
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -R

echo -n "Trying to get another user's key/value.  Expect BADKEY     "
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -T makefile -v getkey.file

echo -n "Setting bob's key.  Expect OK                              "
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -I bobkey -v $file3

echo -n "Getting bob's key/value.  Expect OK                        "
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -T bobkey -v getkey.file
diff $file3 getkey.file
rm -f getkey.file

echo -n "Instructing server to persist data.  Expect OK             "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -V

echo "----------------------------------------------------------------"

echo "Stopping server.  Expect Server terminated and OK             "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -B

echo ""

echo "Re-starting server to check that data persisted correctly"
./obj64/server.exe -t $numthreads -p 9999 -k rsa -f company.dir > /dev/null &
sleep 1

echo -n "Getting alice's key/value.  Expect OK                      "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -T makefile -v getkey.file
diff $file1 getkey.file
rm -f getkey.file

echo -n "Getting bob's key/value.  Expect OK                        "
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -T bobkey -v getkey.file
diff $file3 getkey.file
rm -f getkey.file

echo -n "Setting bob's 2nd key/value.  Expect OK                    "
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -I bk1 -v $file1
echo -n "Setting bob's 3rd key/value.  Expect OK                    "
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -I bk2 -v $file2
echo -n "Setting bob's 4th key/value.  Expect OK                    "
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -I bk3 -v $file4

echo -n "Instructing server to persist data.  Expect OK             "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -V
echo -n "Instructing server to persist data again.  Expect OK       "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -V

echo "----------------------------------------------------------------"

echo "Stopping server.  Expect Server terminated and OK             "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -B

echo ""

echo "Re-starting server to check that data persisted correctly"
./obj64/server.exe -t $numthreads -p 9999 -k rsa -f company.dir > /dev/null &
sleep 1

echo -n "Getting bob's 1st key/value.  Expect OK                    "
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -T bobkey -v getkey.file
diff $file3 getkey.file
rm -f getkey.file
echo -n "Getting bob's 2nd key/value.  Expect OK                    "
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -T bk1 -v getkey.file
diff $file1 getkey.file
rm -f getkey.file
echo -n "Getting bob's 3rd key/value.  Expect OK                    "
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -T bk2 -v getkey.file
diff $file2 getkey.file
rm -f getkey.file
echo -n "Getting bob's 4th key/value.  Expect OK                    "
obj64/client.exe -k localhost.pub -u bob -w bob_is_the_best -s localhost -p 9999 -T bk3 -v getkey.file
diff $file4 getkey.file
rm -f getkey.file

echo "----------------------------------------------------------------"

echo "Stopping server.  Expect Server terminated and OK             "
obj64/client.exe -k localhost.pub -u alice -w alice_is_awesome -s localhost -p 9999 -B

# Reset file system
rm rsa.pri rsa.pub company.dir # server makes these
rm localhost.pub               # client makes these
