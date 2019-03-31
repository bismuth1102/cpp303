# Assignment #2: From Directory to Concurrent Key/Value Store

In this step of the assignment, we will extend our company directory server in
two ways.  The first is that we will make it concurrent.  The second is that we
will transform it from a simple directory into a key/value store.

## Key/Value Store

Right now, your implementation has a `Directory` object, which is really just a
`std::map` holding user metadata and contents.  We will extend the `Directory`
by giving each `Entry` one more field, called `kvstore`.  The `kvstore` will,
itself, be a `std::map`, from `std::string` to `buf_t`.  The key should be at
most 128 bytes.  The value should be at most 1M bytes.  Each user should be able
to put as may key/value pairs into their own map, but not have the ability to
access anyone else's map.

To support this new functionality, we will need to extend our protocol with four
new commands:
   * `KEYINS` -- Insert -- Takes a key and value as its parameters, and attempts
     to create a mapping from that key to that value.  It returns OK if mapping
     could be created, and BADKEY if the key was already present in the map.
     Note: Trying to insert for a key that is already present is an error.
   * `KEYGET` -- Get -- Takes a key as its parameter, and attempts to get the
     value for that key.  It returns OK\nlength\nvalue if the key was present,
     and BADKEY if the key does not exist.
   * `KEYDEL` -- Delete -- Takes a key as its parameter, and attempts to delete
     the existing mapping for that key.  It returns OK if the key was present,
     and BADKEY if the key did not exist.  Note: trying to delete a
     nonexistent key is an error.
   * `KEYUPD` -- Update -- Takes a key and a value as its parameters, and
     attempts to replace the existing mapping for that key with a new mapping.
     It returns OK if the key already existed, and BADKEY if the key did not
     exist.  Note: trying to update for a nonexistent key is an error.

Note: protocol.h has more information about these messages.  You will need to
implement them in the client and the server.

Note: be sure to read client_args.h for details about how the client should
interact with the server.  Your client should read from files to get the keys
and values it sends.

## Concurrency

There are two tasks in order to make the new server concurrent.  The first is
that you must make it possible for threads to handle client connections
simultaneously.  The way to do this is via a _thread pool_.  The server will
take an additional parameter for the number of threads in this pool (see
`server_args.h`).  Whenever a new connection arrives, the server should put the
corresponding file descriptor into a shared queue.  Threads in the pool should
use condition variables to wait until there is an element in the queue, at which
time they can extract it and serve the corresponding client.

In addition to a thread pool, your server will need to make sure that its data
structures are thread-safe.  Since the `Directory` is currently a `std::map`,
the natural solution is to replace it with an array of vectors.  In this
manner, each list can be locked independently, which will allow threads to
simultaneously access the `Directory` to find the `Entry` corresponding to the
client making a request.  For most requests, only one linked list will need to
be locked.  However, an `ALL` command will need some strategy for achieving
atomicity (i.e., it needs two-phase locking).  The number of buckets in the
`Directory` will be a constant for now.

Note that your thread pool will need to handle the `BYE` command very carefully.

## Persistence

Persisting the `Directory` is going to be much more challenging now that we have
concurrency.  For now, we will use a special command, `SAVE`.  That is,
persistence should only happen when the client sends `SAVE`.  As with `ALL`,
this will require two-phase locking.  It will also require a more complex binary
format.  See notes throughout the source code for more details.

## How to Start

To start, you should read through the assignment files, paying attention to the
places where there is a comment of the form `// [303]`.