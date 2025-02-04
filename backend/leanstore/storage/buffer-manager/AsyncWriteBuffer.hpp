#pragma once
#include "BufferFrame.hpp"
#include "Units.hpp"
#include "leanstore/storage/bdev/NVMeStorage.hpp"
// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
#include <libaio.h>
#include <libxnvme.h>
#include <functional>
// -------------------------------------------------------------------------------------
namespace leanstore
{
namespace storage
{
// -------------------------------------------------------------------------------------
class AsyncWriteBuffer
{
  private:
   struct WriteCommand {
      BufferFrame* bf;
      PID pid;
   };

   // NVMe device and queue
   struct xnvme_dev* dev = nullptr;
   struct xnvme_queue* queue = nullptr;

   // Aligned write buffer
   //BufferFrame::Page* write_buffer = nullptr;

   // Commands and metadata
   //std::unique_ptr<WriteCommand[]> write_buffer_commands;

   // Configuration
   uint64_t page_size;
   uint64_t batch_max_size;

   // State
   uint64_t pending_requests = 0;
   //io_context_t aio_context;
   xnvme_cmd_ctx ctx;
   leanstore::storage::bdev::NVMeStorage* storage;

  public:
   std::unique_ptr<BufferFrame::Page[]> write_buffer;
   std::unique_ptr<WriteCommand[]> write_buffer_commands;
   //std::unique_ptr<struct iocb[]> iocbs;
   //std::unique_ptr<struct iocb*[]> iocbs_ptr;
   //std::unique_ptr<struct io_event[]> events;
   // -------------------------------------------------------------------------------------
   // Debug
   // -------------------------------------------------------------------------------------
   // AsyncWriteBuffer(leanstore::storage::bdev::NVMeStorage* storage, u64 page_size, u64 batch_max_size);
   AsyncWriteBuffer(const char* device_path, u64 page_size, u64 batch_max_size);
   ~AsyncWriteBuffer();
   // Caller takes care of sync
   bool full();
   void add(BufferFrame& bf, PID pid);
   u64 submit();
   u64 pollEventsSync();
   void getWrittenBfs(std::function<void(BufferFrame&, u64, PID)> callback, u64 n_events);
};
// -------------------------------------------------------------------------------------
}  // namespace storage
}  // namespace leanstore
// -------------------------------------------------------------------------------------
