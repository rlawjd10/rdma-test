#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <infiniband/verbs.h>

// Similar setup code as the server...

int main() {
    struct ibv_device **dev_list;
    struct ibv_device *ib_dev;
    struct ibv_context *context;
    struct ibv_pd *pd;
    struct ibv_mr *mr;
    struct ibv_qp *qp;
    struct ibv_cq *cq;
    struct ibv_comp_channel *comp_chan;
    void *buf;
    int num_devices;

    // Get the list of devices
    dev_list = ibv_get_device_list(&num_devices);
    if (!dev_list) {
        perror("Failed to get IB devices list");
        return 1;
    }

    // Select the first device
    ib_dev = dev_list[0];
    context = ibv_open_device(ib_dev);

    // Allocate Protection Domain
    pd = ibv_alloc_pd(context);

    // Allocate memory and register it
    buf = malloc(4096);
    mr = ibv_reg_mr(pd, buf, 4096, IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE);

    // Create Completion Queue
    comp_chan = ibv_create_comp_channel(context);
    cq = ibv_create_cq(context, 1, NULL, comp_chan, 0);

    // Create Queue Pair
    struct ibv_qp_init_attr qp_init_attr = {
        .send_cq = cq,
        .recv_cq = cq,
        .cap     = {
            .max_send_wr  = 1,
            .max_recv_wr  = 1,
            .max_send_sge = 1,
            .max_recv_sge = 1,
        },
        .qp_type = IBV_QPT_RC
    };
    qp = ibv_create_qp(pd, &qp_init_attr);

    // Set QP to INIT
    struct ibv_qp_attr qp_attr = {
        .qp_state        = IBV_QPS_INIT,
        .pkey_index      = 0,
        .port_num        = PORT_NUM,
        .qp_access_flags = IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE
    };
    ibv_modify_qp(qp, &qp_attr,
                  IBV_QP_STATE | IBV_QP_PKEY_INDEX |
                  IBV_QP_PORT | IBV_QP_ACCESS_FLAGS);

    // More code to establish connection and perform RDMA operations...

    // Clean up
    ibv_destroy_qp(qp);
    ibv_destroy_cq(cq);
    ibv_dealloc_pd(pd);
    ibv_close_device(context);
    ibv_free_device_list(dev_list);
    free(buf);

    return 0;
}
