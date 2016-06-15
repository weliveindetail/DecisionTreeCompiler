#pragma once

#include <string>

#include <llvm/ExecutionEngine/ObjectCache.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>

// This object cache implementation writes cached objects to disk to the
// directory specified by CacheDir, using a filename provided in the module
// descriptor. The cache tries to load a saved object using that path if the
// file exists. CacheDir defaults to "", in which case objects are cached
// alongside their originating bitcodes.

class SimpleObjectCache : public llvm::ObjectCache {
public:
  SimpleObjectCache(std::string CacheDir = "") : CacheDir(CacheDir) {
    // Add trailing '/' to cache dir if necessary.
    if (!this->CacheDir.empty() &&
        this->CacheDir[this->CacheDir.size() - 1] != '/')
      this->CacheDir += '/';
  }
  ~SimpleObjectCache() override {}

  void notifyObjectCompiled(const llvm::Module *M,
                            llvm::MemoryBufferRef Obj) override {
    const std::string ModuleID = M->getModuleIdentifier();
    std::string CacheName;
    if (!getCacheFilename(ModuleID, CacheName))
      return;
    if (!CacheDir.empty()) { // Create user-defined cache dir.
      llvm::SmallString<128> dir(llvm::sys::path::parent_path(CacheName));
      llvm::sys::fs::create_directories(llvm::Twine(dir));
    }
    std::error_code EC;
    llvm::raw_fd_ostream outfile(CacheName, EC, llvm::sys::fs::F_None);
    outfile.write(Obj.getBufferStart(), Obj.getBufferSize());
    outfile.close();
  }

  std::unique_ptr<llvm::MemoryBuffer>
  getObject(const llvm::Module *M) override {
    const std::string ModuleID = M->getModuleIdentifier();
    std::string CacheName;
    if (!getCacheFilename(ModuleID, CacheName))
      return nullptr;
    // Load the object from the cache filename
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> IRObjectBuffer =
        llvm::MemoryBuffer::getFile(CacheName.c_str(), -1, false);
    // If the file isn't there, that's OK.
    if (!IRObjectBuffer)
      return nullptr;
    // MCJIT will want to write into this buffer, and we don't want that
    // because the file has probably just been mmapped.  Instead we make
    // a copy.  The filed-based buffer will be released when it goes
    // out of scope.
    return llvm::MemoryBuffer::getMemBufferCopy(
        IRObjectBuffer.get()->getBuffer());
  }

  bool getCacheFilename(const std::string &ModID, std::string &CacheName) {
    std::string Prefix("file:");
    size_t PrefixLength = Prefix.length();
    if (ModID.substr(0, PrefixLength) != Prefix)
      return false;
    std::string CacheSubdir = ModID.substr(PrefixLength);

#if defined(_WIN32)
    // Transform "X:\foo" => "/X\foo" for convenience.
    if (isalpha(CacheSubdir[0]) && CacheSubdir[1] == ':') {
      CacheSubdir[1] = CacheSubdir[0];
      CacheSubdir[0] = '/';
    }
#endif

    CacheName = CacheDir + CacheSubdir;
    size_t pos = CacheName.rfind('.');
    CacheName.replace(pos, CacheName.length() - pos, ".o");
    return true;
  }

private:
  std::string CacheDir;
};
