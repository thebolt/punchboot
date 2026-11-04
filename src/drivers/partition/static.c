#include <drivers/partition/static.h>
#include <pb/bio.h>
#include <pb/pb.h>

int static_ptbl_init(bio_dev_t dev, const struct static_part_table *table)
{
    ssize_t rc = bio_block_size(dev);
    lba_t next_start_lba = 0;
    const struct static_part_table *entry = table;

    if (table == NULL)
        return -PB_ERR_PARAM;
    if (rc <= 0)
        return (rc < 0) ? (int)rc : -PB_ERR_PARAM;

    size_t block_sz = (size_t)rc;

    for (; entry->uu != NULL; entry++) {
        if (entry->first_lba) {
            /* In order: an earlier start would overlap the previous one */
            if (entry->first_lba < next_start_lba)
                return -PB_ERR_PARAM;
            next_start_lba = entry->first_lba;
        }

        if (entry->size % block_sz != 0) {
            return -PB_ERR_ALIGN;
        }

        /* Not empty, and the last LBA fits in an lba_t without a wrap */
        uint64_t n_blocks = entry->size / block_sz;
        if (n_blocks == 0)
            return -PB_ERR_PARAM;
        uint64_t last = (uint64_t)next_start_lba + n_blocks - 1;
        if (last > UINT32_MAX)
            return -PB_ERR_PARAM;

        lba_t last_lba = (lba_t)last;
        bio_dev_t new_dev = bio_allocate_parent(
            dev, next_start_lba, last_lba, block_sz, entry->uu, entry->description);
        if (new_dev < 0) {
            LOG_ERR("bio alloc failed (%i)", new_dev);
            return new_dev;
        }

        next_start_lba = last_lba + 1;
    }

    return PB_OK;
}
