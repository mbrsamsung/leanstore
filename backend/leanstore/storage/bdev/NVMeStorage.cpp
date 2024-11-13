#include "NVMeStorage.hpp"

namespace leanstore
{
namespace storage
{
namespace bdev
{


int setup_xnvme(char* dev_uri)
{
    /* Setup xnvme device
     */
    struct xnvme_opts opts = xnvme_opts_default();
    struct xnvme_dev* dev = NULL;
    uint32_t nsid;

    struct xnvme_queue* queue = NULL;
    const int queue_depth = 16;
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
    return 0;
}


int cleanup(struct xnvme_dev* dev, char* buf, xnvme_queue* queue)
{
    int ret;
    ret = xnvme_queue_drain(queue);
	if (ret < 0) {
		xnvme_cli_perr("xnvme_queue_drain()", ret);
		return -1;
	}
    xnvme_buf_free(dev, buf);
    xnvme_queue_term(queue);
    xnvme_dev_close(dev);
    return 0;
}



}  // namespace bdev
}  // namespace storage
}