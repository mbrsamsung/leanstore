#include "NVMeStorage.hpp"

namespace leanstore
{
namespace storage
{
namespace bdev
{

NVMeStorage::NVMeStorage(const char* dev_uri)
{
    /**
     * NVMe Storage Constructor to setup xnvme
     */
    setup_xnvme(dev_uri);
}

NVMeStorage::~NVMeStorage()
{
    /**
     * NVMe Storage Destructor to setup xnvme
     */
    cleanup();
}

int
NVMeStorage::setup_xnvme(const char* dev_uri)
{
    /**
     * Setup xnvme device
     */
    struct xnvme_opts opts = xnvme_opts_default();
    dev = NULL;
    queue = NULL;
    nsid = NULL;

    const int queue_depth = 16;
    //size_t buf_nbytes = xnvme_dev_get_geo(dev)->nbytes;

    int ret = 0, err = 0;
    dev = xnvme_dev_open(dev_uri, &opts);
    nsid = xnvme_dev_get_nsid(dev);

    // Initialize a command-queue
    ret = xnvme_queue_init(dev, queue_depth, 0, &queue);

   	if (ret) {
		xnvme_cli_perr("xnvme_queue_init()", ret);
		xnvme_dev_close(dev);
		return 1;
	}
    return ret;
}

char*
NVMeStorage::nvme_random_buffer(uint64_t buf_nbytes)
{
    //Allocate xnvme buffer, dont zero it out
    char* buf = static_cast<char*>(xnvme_buf_alloc(dev, buf_nbytes));

	if (!buf) {
		xnvme_cli_perr("xnvme_buf_alloc()", errno);
        cleanup();
	}
    return buf;
}

int
NVMeStorage::nvme_write(void* buf, uint64_t buff_size, uint64_t buff_offset)
{
    /**
     * Write
     * - buf, buffer containing data to be written can be arbitrary large
     * - buff_size total buffer size
     * - buff_offset of the part of the buffer that should be written in this call
     */
    int err;
    uint64_t slba, chunks;
    uint16_t nlba;
    size_t written;
    struct xnvme_cmd_ctx ctx = xnvme_cmd_ctx_from_dev(dev);
    size_t lba_nbytes = xnvme_dev_get_geo(dev)->lba_nbytes;
    size_t mdts_bytes = xnvme_dev_get_geo(dev)->mdts_nbytes;


    // Number of LBA's

    chunks = buff_size / mdts_bytes;
    written = 0;

    //std::cout << "NUMBER OF CHUNKS: " << chunks << " / MAX SIZE BYTES: " << mdts_bytes << " / LBA Size: " << lba_nbytes << " / Buffer size: " << buff_size << std::endl;

    for (size_t i = 0; i <= chunks; i++) {
        slba = (i * mdts_bytes + buff_offset) / lba_nbytes;
        nlba = XNVME_MIN((mdts_bytes / lba_nbytes), (buff_size - written) / lba_nbytes) - 1;
        void * data = buf + i * mdts_bytes;
        //std::cout << "SLBA! " << slba << " / NLBA: " << nlba << std::endl;
        err = xnvme_nvm_write(&ctx, nsid, slba, nlba, data, NULL);

        written += (nlba + 1) * lba_nbytes;

        if (err || xnvme_cmd_ctx_cpl_status(&ctx)) {
           xnvme_cli_perr("xnvme_nvm_write()", err);
           xnvme_cmd_ctx_pr(&ctx, XNVME_PR_DEF);
           err = err ? err : -EIO;
           return -1;
        }
    }

    xnvme_cli_pinf("Submitted and completed write command succesfully");
    std::cout << "Written: " << written << std::endl;
    return written;
}

int
NVMeStorage::nvme_read(void* buf, uint64_t buff_size, uint64_t buff_offset)
{
    /**
     * Read sync
     */
    struct xnvme_cmd_ctx ctx = xnvme_cmd_ctx_from_dev(dev);
    int err;

    // Submit and wait for the completion of a read command
    err = xnvme_nvm_read(&ctx, nsid, 0x0, 0, buf, NULL);
    if (err || xnvme_cmd_ctx_cpl_status(&ctx))
    {
       xnvme_cli_perr("xnvme_nvm_read()", err);
       xnvme_cmd_ctx_pr(&ctx, XNVME_PR_DEF);
       err = err ? err : -EIO;
       cleanup();
       return -1;
    }
    xnvme_cli_pinf("Submitted and completed command succesfully");
    return 0;
}

int
NVMeStorage::cleanup()
{
   int ret;
   ret = xnvme_queue_drain(queue);
   if (ret < 0) {
      xnvme_cli_perr("xnvme_queue_drain()", ret);
      return -1;
   }
   //xnvme_buf_free(dev, buf);
   xnvme_queue_term(queue);
   xnvme_dev_close(dev);
   return 0;
}

}  // namespace bdev
}  // namespace storage
}