#pragma once
#include <errno.h>
#include <libxnvme.h>
#include <iostream>


namespace leanstore
{
namespace storage
{
namespace bdev
{

class NVMeStorage
{
private:
    struct xnvme_dev *dev;
    struct xnvme_queue *queue;
    uint32_t nsid;

public:
    NVMeStorage(const char* dev_uri);
    ~NVMeStorage();

    int setup_xnvme(const char* dev_uri);

    char* nvme_random_buffer(uint64_t buf_nbytes);

    int nvme_write(void* buf, uint64_t buff_size, uint64_t buff_offset);

    int nvme_read(void* buf, uint64_t buff_size, uint64_t buff_offset);

    int cleanup();
};

}
}  // namespace storage
}  // namespace leanstore