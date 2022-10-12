/*
 * Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "hydra_server.h"
#include "hydra.h"
#include "pmiserv.h"
#include "pmiserv_pmi.h"
#include "pmiserv_utils.h"
#include "bsci.h"

HYD_status HYD_pmiserv_barrier(int fd, int pid, int pgid, struct PMIU_cmd *pmi)
{
    struct HYD_proxy *proxy, *tproxy;
    HYD_status status = HYD_SUCCESS;

    HYDU_FUNC_ENTER();

    proxy = HYD_pmcd_pmi_find_proxy(fd);
    HYDU_ASSERT(proxy, status);

    proxy->pg->barrier_count++;
    if (proxy->pg->barrier_count == proxy->pg->proxy_count) {
        proxy->pg->barrier_count = 0;

        HYD_pmiserv_bcast_keyvals(fd, pid);

        struct PMIU_cmd pmi_response;
        PMIU_cmd_init_static(&pmi_response, 1, "barrier_out");
        for (tproxy = proxy->pg->proxy_list; tproxy; tproxy = tproxy->next) {
            status = HYD_pmiserv_pmi_reply(tproxy->control_fd, pid, &pmi_response);
            HYDU_ERR_POP(status, "error writing PMI line\n");
        }
    }

  fn_exit:
    HYDU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

HYD_status HYD_pmiserv_abort(int fd, int pid, int pgid, struct PMIU_cmd *pmi)
{
    HYD_status status = HYD_SUCCESS;
    int pmi_errno;

    HYDU_FUNC_ENTER();

    int exitcode;
    const char *error_msg;
    pmi_errno = PMIU_msg_get_query_abort(pmi, &exitcode, &error_msg);
    HYDU_ASSERT(!pmi_errno, status);

  fn_exit:
    /* clean everything up and exit */
    status = HYDT_bsci_wait_for_completion(0);
    exit(exitcode);

    /* never get here */
    HYDU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}