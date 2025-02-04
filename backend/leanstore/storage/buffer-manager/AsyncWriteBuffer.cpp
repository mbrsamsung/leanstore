#include "AsyncWriteBuffer.hpp"
#include "Tracing.hpp"
#include "Exceptions.hpp"
#include "leanstore/profiling/counters/WorkerCounters.hpp"
// -------------------------------------------------------------------------------------
#include "gflags/gflags.h"

// -------------------------------------------------------------------------------------
#include <signal.h>
#include <cstring>
#include <stdexcept>
// -------------------------------------------------------------------------------------
DEFINE_uint32(insistence_limit, 1, "");

namespace leanstore
{
namespace storage
{
// -------------------------------------------------------------------------------------
AsyncWriteBuffer::AsyncWriteBuffer(const char* device_path, u64 page_size, u64 batch_max_size)
    : page_size(page_size), batch_max_size(batch_max_size)
{
   // Open NVMe device
   struct xnvme_opts opts = xnvme_opts_default();
   dev = xnvme_dev_open(device_path, &opts);
   ctx = xnvme_cmd_ctx_from_dev(dev);
   if (!dev) {
      throw ex::GenericException("Failed to open NVMe device: " + std::string(device_path));
   }

   // Initialize an NVMe I/O queue
   if (xnvme_queue_init(dev, batch_max_size, 0, &queue) < 0) {
      throw ex::GenericException("Failed to initialize NVMe queue");
   }

   // Allocate aligned buffers for write operations
   write_buffer = make_unique<BufferFrame::Page[]>(batch_max_size);

   if (!write_buffer) {
      throw ex::GenericException("Failed to allocate write buffer");
   }
   write_buffer_commands = make_unique<WriteCommand[]>(batch_max_size);
}
// -------------------------------------------------------------------------------------
bool AsyncWriteBuffer::full()
{
   return pending_requests >= batch_max_size - 2;
}
// -------------------------------------------------------------------------------------
void AsyncWriteBuffer::add(BufferFrame& bf, PID pid)
{
   assert(!full());
   assert(reinterpret_cast<u64>(&bf.page) % 512 == 0);
   assert(pending_requests <= batch_max_size);

   COUNTERS_BLOCK() { WorkerCounters::myCounters().dt_page_writes[bf.page.dt_id]++; }

   // -------------------------------------------------------------------------------------
   auto slot = pending_requests++;
   write_buffer_commands[slot].bf = &bf;
   write_buffer_commands[slot].pid = pid;
   bf.page.magic_debugging_number = pid;

   // Copy page to aligned buffer
   std::memcpy(&write_buffer[slot], &bf.page, page_size);

   // Submit a write command
   int ret = xnvme_nvm_write(&ctx, &write_buffer[slot], pid * page_size, page_size, NULL);
   if (ret < 0) {
      throw ex::GenericException("xnvme_cmd_pwrite failed");
   }
}
// -------------------------------------------------------------------------------------
u64 AsyncWriteBuffer::submit()
{
   if (pending_requests > 0) {
      // Submit all pending requests
      int completed = xnvme_queue_poke(queue, pending_requests);
      if (completed < static_cast<int>(pending_requests)) {
         throw ex::GenericException("xnvme_queue_poke failed to submit all requests");
      }
      return completed;
   }
   return 0;
}
// -------------------------------------------------------------------------------------
u64 AsyncWriteBuffer::pollEventsSync()
{
   if (pending_requests > 0) {
      // Wait for all pending requests to complete
      int completed = xnvme_queue_drain(queue);
      if (completed != static_cast<int>(pending_requests)) {
         raise(SIGTRAP);
         throw ex::GenericException("xnvme_queue_wait did not complete all requests");
      }
      pending_requests = 0;
      return completed;
   }
   return 0;
}
// -------------------------------------------------------------------------------------
void AsyncWriteBuffer::getWrittenBfs(std::function<void(BufferFrame&, u64, PID)> callback, u64 n_events)
{
   for (u64 i = 0; i < n_events; ++i) {
      const auto slot = i;
      auto written_lsn = write_buffer[slot].PLSN;
      callback(*write_buffer_commands[slot].bf, written_lsn, write_buffer_commands[slot].pid);
   }
}
// -------------------------------------------------------------------------------------
AsyncWriteBuffer::~AsyncWriteBuffer()
{
   // Clean up resources
   if (queue) {
      xnvme_queue_term(queue);
   }
   if (dev) {
      xnvme_dev_close(dev);
   }
}
// -------------------------------------------------------------------------------------
}  // namespace storage
}  // namespace leanstore
