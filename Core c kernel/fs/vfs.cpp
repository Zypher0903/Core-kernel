#include <kernel/fs/vfs.h>
#include <kernel/console.h>

namespace Core {

void VFS::initialize() {
    Console::printf("[VFS] Virtual File System initialized\n");
}

}