# command-matcher

Command finder for smart-home–style projects. It exposes a small C API in a shared library (`libmanulu.so`) that you can call from C, Java, Python or via PHP FFI to parse/process natural-language commands and read back structured results.

## Features

- Written entirely in **C** (compact, no external dependencies)  
- Compiles into a small shared library (`libmanulu.so`, ~2.3 MB binary size). Estimated memory usage ≈ 2.8 MB in resident set  
- Minimal CPU requirements (runs comfortably on ≥ 433 MHz processors)  
- Uses `/static/index.db` (**SQLite database**) for all command definitions. No coding required to add/change commands  
- Fully integrated with **Uzbek language morphology** for accurate parsing  
- **Macros support** for transforming regex matches before inserting into JSON
- Provides a **Java JNI interface** for Android/Java projects  
- Optional **Python** and **PHP-FFI** wrappers for scripting/quick integration 

## Supporting author

- [tirikchilik](https://tirikchilik.uz/yetimdasturchi)
- [buymeacoffee](https://www.buymeacoffee.com/yetimdasturchi)
- [github](https://github.com/sponsors/yetimdasturchi)

## Requirements

### Linux

-   GCC (or Clang)
-   `make`
    

### Android (optional)

-   Android NDK
-   `ANDROID_NDK_HOME` environment variable set


## Build (Linux)

```bash
# from repo root
make            # same as `make linux`
# artifacts:
#   out/libmanulu.so
#   out/test
```

Rebuild from scratch:

```bash
make clean && make
```

Run the C test harness:

```bash
make test
``` 

----------

## Build (Android NDK)

Set up `ANDROID_NDK_HOME` and run:

`export ANDROID_NDK_HOME=/path/to/android-ndk
make android` 
-   Outputs go to the standard NDK `libs/` & `obj/` folders.
-   Adjust `APP_PLATFORM` (currently `android-29`) in the Makefile if you target another API level.


## Using the C API

### Exported functions

```c
int init_res(const char* base_path);
int get_from_text(const char* input);

const char* get_domain(void);
const char* get_operational_label(void);
const char* get_original(void);
const char* get_answer(void);
const char* get_json(void);
``` 

Typical flow:

1.  `init_res("./static/")` — initialize once
2.  `get_from_text("turn on led")` — process an input
3.  Read results via `get_*()` accessors


## JNI bridge

```java
package uz.manu.command.matcher;

public class CommandMatcher {
    static {
        System.loadLibrary("manulu");
    }

    public native int initRes(String dir);
    public native int getFromText(String input);
    public native void unInitRes();

    public native String getDomain();
    public native String getOperationalLabel();
    public native String getIndex();
    public native String getAnswer();
    public native String getJson();

    public static void main(String[] args) {
        CommandMatcher cm = new CommandMatcher();

        int rc = cm.initRes("./static/");
        if (rc != 0) {
            System.err.println("Failed to init!");
            return;
        }

        rc = cm.getFromText("chiroqni yoq");
        if (rc != 0) {
            System.err.println("Processing failed!");
            return;
        }

        System.out.println("Domain: " + cm.getDomain());
        System.out.println("Label: " + cm.getOperationalLabel());
        System.out.println("Index: " + cm.getIndex());
        System.out.println("Answer: " + cm.getAnswer());
        System.out.println("Json: " + cm.getJson());

        cm.unInitRes();
    }
}
```

## PHP-FFI

```php
<?php

namespace Uz\Manu\Command\Matcher;

use FFI;

class CommandMatcher
{
    private FFI $ffi;

    public function __construct(string $libPath)
    {
        $this->ffi = FFI::cdef(<<<'CDEF'
            int init_res(const char* base_path);
            int get_from_text(const char* input);

            const char* get_domain(void);
            const char* get_operational_label(void);
            const char* get_original(void);
            const char* get_answer(void);
            const char* get_json(void);
        CDEF, $libPath);
    }

    public function initRes(string $dir): int
    {
        return $this->ffi->init_res($dir);
    }

    public function getFromText(string $input): int
    {
        return $this->ffi->get_from_text($input);
    }

    public function getDomain(): string
    {
        return self::cstr($this->ffi->get_domain());
    }

    public function getOperationalLabel(): string
    {
        return self::cstr($this->ffi->get_operational_label());
    }

    public function getIndex(): string
    {
        return self::cstr($this->ffi->get_original());
    }

    public function getAnswer(): string
    {
        return self::cstr($this->ffi->get_answer());
    }

    public function getJson(): string
    {
        return self::cstr($this->ffi->get_json());
    }

    private static function cstr($val): string
    {
        return ($val instanceof \FFI\CData) ? FFI::string($val) : (string)$val;
    }
}
```

## Minimal C usage

```c
#include <stdio.h>
#include "manulu.h"

int main(void) {
    if (init_res("./static/") != 0) return 1;
    if (get_from_text("chiroqni yoq") != 0) return 1;

    printf("Domain: %s\n", get_domain());
    printf("Label:  %s\n", get_operational_label());
    printf("Orig:   %s\n", get_original());
    printf("Answer: %s\n", get_answer());
    printf("JSON:   %s\n", get_json());
    return 0;
}
```

## Python

```python
import ctypes
import os

LIB_PATH = os.path.join(os.path.dirname(__file__), "out", "libmanulu.so")

lib = ctypes.CDLL(LIB_PATH)

lib.init_res.argtypes = [ctypes.c_char_p]
lib.init_res.restype  = ctypes.c_int

lib.get_from_text.argtypes = [ctypes.c_char_p]
lib.get_from_text.restype  = ctypes.c_int

lib.un_init_res.argtypes = []
lib.un_init_res.restype  = None

lib.get_domain.restype             = ctypes.c_char_p
lib.get_operational_label.restype  = ctypes.c_char_p
lib.get_original.restype           = ctypes.c_char_p
lib.get_answer.restype             = ctypes.c_char_p
lib.get_json.restype               = ctypes.c_char_p

if __name__ == "__main__":
    rc = lib.init_res(b"./static/")
    if rc != 0:
        print("Failed to init!")
        exit(1)

    rc = lib.get_from_text(b"chiroqni yoq")
    if rc != 0:
        print("Processing failed!")
        lib.un_init_res()
        exit(1)

    print("Domain:", lib.get_domain().decode())
    print("Label:", lib.get_operational_label().decode())
    print("Index:", lib.get_original().decode())
    print("Answer:", lib.get_answer().decode())
    print("Json:", lib.get_json().decode())

    lib.un_init_res()
```