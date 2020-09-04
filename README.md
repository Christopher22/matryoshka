# matryoshka
*matryoshka* is a virtual filesystem stored in an SQLite database. It supports features like chunked storage, data streaming, handling of paths, and searching for files. Written in modern C++17, it is carefully tested for correct behavior. Besides this, it is optimized to be accessed in a variety of ways:
- Through a command line interface
- Through Python
- Through C# 
- Through WebDAV (currently in development)

As such, *matryoshka* is the ideal solution for bundling files in a transparent and performant way.