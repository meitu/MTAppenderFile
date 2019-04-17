# MTAppenderFile

A simplified high-performance log component for *OS base on Tencent Mars xlog, is used in [MTHawkeye](https://github.com/meitu/MTHawkeye) for performance record data persistance.

## Usage

Creata a `AppenderFile` by using `Objective-C` API,

```objc
MTAppenderFile *file = [[MTAppenderFile alloc] initWithFileDir:dirName name:fileName];
[file open];
```

Then use `appendText:` or `appendUTF8Text:` to append line data.

```objc
[file appendText:@"test line"];
[file appendUTF8Text:"test line"];
```

## Details

Each `AppenderFile` contains two files with the same file name, and different extensions. The first one is mmap file for high speed cache with the extension `.mmap2`, it's size is 150KB. The other file a extension `.mtlog`, log data transfered to this file when the mmap file needs to be dumped.

For example, if you create an `AppenderFile` whose name is records, the actual files are: `records.mmap2` and `records.mtlog`. You need to merge the two files when reading, which is the complete records.

Attention for reading stored data:

- Both files need to be read, first `*.mmap2`, then `*.mtlog`, and then put the content together.
- When reading the `*.mmap2` file, only non-dirty data is needed. when the `\0\0\0` line is encountered, the complete content of `mmap` has been read, and the content after that is dirty data, which should be ignored.

## Contributing

For more information about contributing issues or pull requests, see [Contributing Guide](./Contributing.md)。

## Base On

1. [Tencent-Mars](https://github.com/Tencent/mars)
2. [微信终端跨平台组件 mars 系列（一） - 高性能日志模块xlog](https://mp.weixin.qq.com/s/cnhuEodJGIbdodh0IxNeXQ)
