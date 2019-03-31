#include "server_storage.h"

vector<Directory::hashList*> Directory::col;

void Directory::initCol(){
    if(col.size()==0){
        for(int i=0; i<BUCKETS; i++){
            hashList *row = new hashList(i);
            col.push_back(row);
        }
    }
}

/**
 * Write the entire directory to the file specified by this.filename.  To
 * ensure durability, the directory must be persisted in two steps.  First, it
 * must be written to a temporary file (this.filename.tmp).  Then the
 * temporary file can be renamed to replace the older version of the
 * directory.
 */
void Directory::persist() {

    string new_filename = this->filename + ".tmp";
    FILE *f = fopen(new_filename.c_str(), "w");

    for(auto hashlist: col){
        for(auto e: hashlist->row){
            fwrite("DIRENTRY", sizeof(char), 8, f);

            string username = e.username;
            int username_length = username.length();

            string password = e.pass_hash;
            int password_length = password.length();

            int num_bytes = e.num_bytes;
            unsigned char *bytes = e.bytes;

            fwrite(&username_length, sizeof(int), 1, f);
            fwrite(username.c_str(), sizeof(char), username_length, f);
            fwrite(&password_length, sizeof(int), 1, f);
            fwrite(password.c_str(), sizeof(char), password_length, f);

            if(num_bytes > 0){
                fwrite(&num_bytes, sizeof(int), 1, f);
                fwrite(bytes, sizeof(char), num_bytes, f);
            }

            if(e.map.size()>0){
                for(auto pair : e.map){
                    fwrite("K.VENTRY", sizeof(char), 8, f);

                    string key = pair.first;
                    int key_length = key.length();

                    buf_t value = pair.second;
                    int value_length = value.num;

                    fwrite(&key_length, sizeof(int), 1, f);
                    fwrite(key.c_str(), sizeof(char), key_length, f);
                    fwrite(&value_length, sizeof(int), 1, f);
                    fwrite(value.bytes, sizeof(char), value_length, f);
                }
            }
        }
    }
    fclose(f);

    int result = rename(new_filename.c_str(), this->filename.c_str());
    if(result!=0){
        std::cerr << "rename error" << std::endl;
    }

}

/**
 * Construct an empty directory and specify the file from which the directory
 * should be loaded.  To avoid exceptions and errors in the constructor, the
 * act of loading data is separate from construction.
 */
Directory::Directory(std::string fname) : filename(fname) {}

/**
 * Populate the directory by loading this.filename.  Note that load() begins
 * by clearing the directory, so that when the call is complete, exactly and
 * only the contents of the file are in the directory.
 *
 * @returns False if any error is encountered, and true otherwise
 */
Directory::Entry Directory::loadDirentry(FILE *f){
    char name_buf[65] = {0}, pass_buf[129] = {0};
    unsigned size = 0;
    Entry err(-1);

    // Read the fields
    if (1 != fread(&size, sizeof(int), 1, f)) { //4 bytes
        return err;
    }
    if (size != fread(name_buf, sizeof(char), size, f)) { //size bytes
        return err;
    }
    std::string name(name_buf);

    if (1 != fread(&size, sizeof(int), 1, f)) { //4 bytes
        return err;
    }
    if (size != fread(pass_buf, sizeof(char), size, f)) { //size bytes
        return err;
    }
    std::string pass(pass_buf);

    if (1 != fread(&size, sizeof(int), 1, f)) { //4 bytes
        return err;
    }
    unsigned char *byte_buf = nullptr;
    if (size) {
        byte_buf = (unsigned char *)malloc(size);
        if (size != fread(byte_buf, sizeof(char), size, f)) { //size bytes
            return err;
        }
    }

    Entry e = {name, pass, (int)size, byte_buf};
    return e;
}

pair<string, buf_t> Directory::loadK_Ventry(FILE *f, Entry e){
    char key_buf[129] = {0}, val_buf[1048577] = {0};
    unsigned size = 0;
    pair<string, buf_t> err("", buf_t(0));

    if (1 != fread(&size, sizeof(int), 1, f)) { //4 bytes
        return err;
    }
    if (size != fread(key_buf, sizeof(char), size, f)) { //size bytes
        return err;
    }
    string key(key_buf);

    if (1 != fread(&size, sizeof(int), 1, f)) { //4 bytes
        return err;
    }
    if (size != fread(val_buf, sizeof(char), size, f)) { //size bytes
        return err;
    }
    
    unsigned char* value_buf = (unsigned char*)malloc(size);
    memcpy(value_buf, val_buf, size);
    buf_t value(size, value_buf);

    pair<string, buf_t> res(key, value);
    return res;
}

bool Directory::load() {
    initCol();

    Entry e(-1);
    unordered_map<string, buf_t> map;

    FILE *f = fopen(filename.c_str(), "r");
    if (f == nullptr) {
        std::cerr << "File not found: " << filename << std::endl;
        return false;
    }

    char pack_buf[9] = {0};
    while (fread(pack_buf, sizeof(char), 8, f)) {
        string packet(pack_buf);

        if (packet == "DIRENTRY") {
            if(e.num_bytes != -1){  //not the first time
                //init map and save at first
                e.map = map;
                map.clear();

                hash<string> hasher;
                size_t username_hash_index = hasher(e.username) % BUCKETS;
                col[username_hash_index]->row.push_back(e);
            }
            e = loadDirentry(f);
            if(e.num_bytes==-1){
                std::cerr << "File read error\n";
                return false;
            }
        }
        else if(packet == "K.VENTRY"){
            pair<string, buf_t> p = loadK_Ventry(f, e);
            if(p.first == ""){
                std::cerr << "File read error\n";
                return false;
            }
            map.insert(p);
        }
        else{
            cerr << "Invalid packet type: " << packet << endl;
            return false;
        }

    }

    fclose(f);
    return true;

}

/**
 * Create a new entry in the Directory.  If the user_name already exists, we
 * should return an error.  Otherwise, hash the password, and then save an
 * entry with the username, hashed password, and a zero-byte content.
 *
 * @param user_name The user name to register
 * @param pass  The password to associate with that user name
 *
 * @returns False if the username already exists, true otherwise
 */
bool Directory::add_user(std::string user_name, std::string pass) {

    hash<string> hasher;
    std::size_t username_hash_index = hasher(user_name) % BUCKETS;

    col[username_hash_index]->lock.lock();

    // If the user exists, fail
    for(auto e : col[username_hash_index]->row){
        if(e.username == user_name){
            col[username_hash_index]->lock.unlock();
            return false;
        }
    }

    // Remember: never store passwords.  Store their hashes instead
    unsigned char result[MD5_DIGEST_LENGTH + 1] = {0};
    MD5((const unsigned char *)pass.c_str(), pass.length(), result);

    // Be sure to persist the directory since we are modifying it
    Entry e(user_name, string((char *)result));
    col[username_hash_index]->row.push_back(e);

    col[username_hash_index]->lock.unlock();
    return true;
}

/**
 * Set the data bytes for a user, but do so if and only if the password
 * matches
 *
 * @param user_name The name of the user whose content is being set
 * @param pass The password for the user, used to authenticate
 * @param num_bytes The length of the array of bytes being provided
 * @param bytes The actual bytes to add to the user's Entry
 *
 * @returns A buf_t with a message that is the result of the attempt
 */
buf_t Directory::set_user_data(std::string user_name, std::string pass, int num_bytes,
                               unsigned char *bytes) {

    hash<string> hasher;
    std::size_t username_hash_index = hasher(user_name) % BUCKETS;
    
    col[username_hash_index]->lock.lock();

    unsigned char result[MD5_DIGEST_LENGTH + 1] = {0};
    MD5((const unsigned char *)pass.c_str(), pass.length(), result);

    for (vector<Entry>::iterator it = col[username_hash_index]->row.begin() ; it != col[username_hash_index]->row.end(); ++it){
        if(it->username == user_name){
            if(it->pass_hash == string((char *)result)){
                unsigned char *data = (unsigned char *)malloc(num_bytes);
                memcpy(data, bytes, num_bytes);
                it->num_bytes = num_bytes;
                it->bytes = data;

                col[username_hash_index]->lock.unlock();
                return buf_t(RES_OK);
            }
            else{
                col[username_hash_index]->lock.unlock();
                return buf_t(RES_ERR_LOGIN);
            }

        }
    }

    //end of for loop, means not found the username
    col[username_hash_index]->lock.unlock();
    return buf_t(RES_ERR_USER_EXISTS);
}

/**
 * Return a copy of the user data for a user, but do so only if the password
 * matches
 *
 * @param user_name The name of the user who made the request
 * @param pass The password for the user, used to authenticate
 * @param who The name of the user whose content is being fetched
 *
 * @returns A buf_t with the data, or a buf_t with a negative number on
 *          failure.  The num will be -1 for invalid user_name, -2 for bad
 *          password, and -3 for invalid who parameter.  Note that num can be
 *          0 if the user has no data.
 */
buf_t Directory::get_user_data(std::string user_name, std::string pass,
                               std::string who) {

    hash<string> hasher;
    std::size_t username_hash_index = hasher(user_name) % BUCKETS;

    col[username_hash_index]->lock.lock();

    // Authenticate the user via password
    unsigned char result[MD5_DIGEST_LENGTH + 1] = {0};
    MD5((const unsigned char *)pass.c_str(), pass.length(), result);

    for (auto e : col[username_hash_index]->row){
        if(e.username == user_name){
            if(e.pass_hash == std::string((char *)result)){

                col[username_hash_index]->lock.unlock();

                std::size_t target_hash_index = hasher(who) % BUCKETS;

                col[target_hash_index]->lock.lock();

                for (auto e2 : col[target_hash_index]->row){
                    if(e2.username == who){
                        unsigned char * bytes = (unsigned char *)malloc(e2.num_bytes + 1);
                        memset(bytes, 0, e2.num_bytes + 1);
                        memcpy(bytes, e2.bytes, e2.num_bytes);

                        buf_t res(e2.num_bytes, bytes);

                        col[target_hash_index]->lock.unlock();
                        return res;
                    }
                }
            }
            else{
                col[username_hash_index]->lock.unlock();
                return buf_t(RES_ERR_LOGIN);
            }
        }
    }
    col[username_hash_index]->lock.unlock();
    return buf_t(RES_ERR_USER_EXISTS);

}

/**
 * Return a newline-delimited string containing all of the usernames in the
 * directory
 *
 * @param user_name The name of the user who made the request
 * @param pass The password for the user, used to authenticate
 *
 * @returns A buf_t with the data, or a buf_t with a negative number on
 *          failure.  The num will be -1 for invalid user_name, or -2 for bad
 *          password
 */
buf_t Directory::get_all_users(std::string user_name, std::string pass) {

    //prevent of deadlock(more than 1 threads run get_all at the same time)
    mutex get_all_mutex;
    get_all_mutex.lock();

    hash<string> hasher;
    std::size_t username_hash_index = hasher(user_name) % BUCKETS;

    // Authenticate the user via password
    unsigned char result[MD5_DIGEST_LENGTH + 1] = {0};
    MD5((const unsigned char *)pass.c_str(), pass.length(), result);

    for (auto e : col[username_hash_index]->row){
        if(e.username == user_name){
            if(e.pass_hash == std::string((char *)result)){

                //lock all rows
                for (int i = 0 ; i<BUCKETS ; ++i){
                    col[i]->lock.lock();
                }

                string all_username;
                buf_t res;
                for (int i = 0 ; i<BUCKETS ; ++i){
                    for(auto e2 : col[i]->row){
                        all_username = all_username + e2.username + "\n";
                    }
                    col[i]->lock.unlock();
                }
                res = buf_t(RES_OK + "\n" + to_string(all_username.length()) + "\n" + all_username);

                get_all_mutex.unlock();
                return res;
            }
            else{
                get_all_mutex.unlock();
                return buf_t(RES_ERR_LOGIN);
            }
        }
    }
    get_all_mutex.unlock();
    return buf_t(RES_ERR_USER_EXISTS);
    
}

/**
 * Perform an insertion into the user's key/value store
 *
 * @param user_name The name of the user who made the request
 * @param pass The password for the user, used to authenticate
 * @param key  The key to insert
 * @param val  The bytes of value to insert
 *
 * @returns A buf_t with a message that is the result of the attempt.
 */
buf_t Directory::kvinsert(std::string user_name, std::string pass, std::string key,
                          buf_t val) {
    hash<string> hasher;
    std::size_t username_hash_index = hasher(user_name) % BUCKETS;
    
    col[username_hash_index]->lock.lock();

    unsigned char result[MD5_DIGEST_LENGTH + 1] = {0};
    MD5((const unsigned char *)pass.c_str(), pass.length(), result);

    //check username and password, then insert key and val.
    for (vector<Entry>::iterator it = col[username_hash_index]->row.begin() ; it != col[username_hash_index]->row.end(); ++it){
        if(it->username == user_name){
            if(it->pass_hash == std::string((char *)result)){
                pair<unordered_map<string,buf_t>::iterator, bool> res;
                res = it->map.insert(pair<string,buf_t>(key, val));

                if(res.second==1){
                    col[username_hash_index]->lock.unlock();
                    return buf_t(RES_OK);
                }
                else{
                    col[username_hash_index]->lock.unlock();
                    return buf_t(RES_ERR_BADKEY);
                }
            }
            else{
                col[username_hash_index]->lock.unlock();
                return buf_t(RES_ERR_LOGIN);
            }
        }
    }

    //end of for loop, means not found the username
    col[username_hash_index]->lock.unlock();
    return buf_t(RES_ERR_USER_EXISTS);  

}

/**
 * Perform an update in the user's key/value store
 *
 * @param user_name The name of the user who made the request
 * @param pass The password for the user, used to authenticate
 * @param key  The key to update
 * @param val  The bytes of value to update
 *
 * @returns A buf_t with a message that is the result of the attempt
 */
buf_t Directory::kvupdate(std::string user_name, std::string pass, std::string key,
                          buf_t val) {

    hash<string> hasher;
    std::size_t username_hash_index = hasher(user_name) % BUCKETS;

    col[username_hash_index]->lock.lock();

    unsigned char result[MD5_DIGEST_LENGTH + 1] = {0};
    MD5((const unsigned char *)pass.c_str(), pass.length(), result);

    for (vector<Entry>::iterator it = col[username_hash_index]->row.begin() ; it != col[username_hash_index]->row.end(); ++it){
        if(it->username == user_name){
            if(it->pass_hash == std::string((char *)result)){

                int exist = it->map.erase(key);

                if(exist){
                    it->map.insert(pair<string,buf_t>(key, val));
                    col[username_hash_index]->lock.unlock();
                    return buf_t(RES_OK);
                }
                else{
                    col[username_hash_index]->lock.unlock();
                    return buf_t(RES_ERR_BADKEY);
                }
            }
            else{
                col[username_hash_index]->lock.unlock();
                return buf_t(RES_ERR_LOGIN);
            }
        }
    }

    //end of for loop, means not found the username
    col[username_hash_index]->lock.unlock();
    return buf_t(RES_ERR_USER_EXISTS);  

}

/**
 * Perform a get from the user's key/value store
 *
 * @param user_name The name of the user who made the request
 * @param pass The password for the user, used to authenticate
 * @param key  The key to get
 *
 * @returns A buf_t with the value, or a buf_t with a negative number on
 *          failure.  The num will be -1 for invalid user_name, -2 for bad
 *          password, and -3 for invalid key.
 */
buf_t Directory::kvget(std::string user_name, std::string pass, std::string key) {
    hash<string> hasher;
    std::size_t username_hash_index = hasher(user_name) % BUCKETS;

    col[username_hash_index]->lock.lock();
    
    unsigned char result[MD5_DIGEST_LENGTH + 1] = {0};
    MD5((const unsigned char *)pass.c_str(), pass.length(), result);

    //check username and password, then insert key and val.
    for (vector<Entry>::iterator it = col[username_hash_index]->row.begin() ; it != col[username_hash_index]->row.end(); ++it){
        if(it->username == user_name){
            if(it->pass_hash == std::string((char *)result)){

                unordered_map<string,buf_t>::iterator it_map = it->map.find(key);
                if (it_map != it->map.end()){
                    col[username_hash_index]->lock.unlock();
                    return it_map->second;
                }
                else{
                    col[username_hash_index]->lock.unlock();
                    return buf_t(-3, nullptr);
                }
                
            }
            else{
                col[username_hash_index]->lock.unlock();
                return buf_t(-2, nullptr);
            }
        }
    }

    col[username_hash_index]->lock.unlock();
    return buf_t(-1, nullptr);
}

/**
 * Perform a delete in the user's key/value store
 *
 * @param user_name The name of the user who made the request
 * @param pass The password for the user, used to authenticate
 * @param key  The key to delete
 *
 * @returns A buf_t with the result of the attempt.
 */
buf_t Directory::kvdel(std::string user_name, std::string pass, std::string key) {
    hash<string> hasher;
    std::size_t username_hash_index = hasher(user_name) % BUCKETS;

    col[username_hash_index]->lock.lock();

    unsigned char result[MD5_DIGEST_LENGTH + 1] = {0};
    MD5((const unsigned char *)pass.c_str(), pass.length(), result);

    for (vector<Entry>::iterator it = col[username_hash_index]->row.begin() ; it != col[username_hash_index]->row.end(); ++it){
        if(it->username == user_name){
            if(it->pass_hash == std::string((char *)result)){

                int exist = it->map.erase(key);
                
                if(exist){
                    col[username_hash_index]->lock.unlock();
                    return buf_t(RES_OK);
                }
                else{
                    col[username_hash_index]->lock.unlock();
                    return buf_t(RES_ERR_BADKEY);
                }
            }
            else{
                col[username_hash_index]->lock.unlock();
                return buf_t(RES_ERR_LOGIN);
            }
        }
    }

    //end of for loop, means not found the username
    col[username_hash_index]->lock.unlock();
    return buf_t(RES_ERR_USER_EXISTS);  
}

/**
 * Public interface for saving the directory to a file
 */
void Directory::save() { 
    mutex save_mutex;
    save_mutex.lock();

    //lock all rows
    for (int i = 0 ; i<BUCKETS ; ++i){
        col[i]->lock.lock();
    }

    persist(); 

    for (int i = 0 ; i<BUCKETS ; ++i){
        col[i]->lock.unlock();
    }
}
