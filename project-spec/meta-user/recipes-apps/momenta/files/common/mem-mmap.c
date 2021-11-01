/** ===================================================== **
 *File : bram-mmap.c
 *Brief : block ram mmap function
 *Author : Momenta founderHAN
 *Created: 2021-3-3
 *Version: 1.0
 ** ===================================================== **/

/** ===================================================== **
 * INCLUDE
 ** ===================================================== **/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include "common.h"

/** ===================================================== **
 * MACRO
 ** ===================================================== **/
#define MEM_MMAP_VERSION "v1.0"

/** ===================================================== **
 * STRUCT
 ** ===================================================== **/

/** ===================================================== **
 * VARIABLE
 ** ===================================================== **/

/** ===================================================== **
 * FUNCTION
 ** ===================================================== **/

/*!
 * \brief block ram mmap open.
 *
 * \param[out] pstBramPtr BramPtr_s*, block ram pointer, include all pointers.
 * \return 0 on success, errors otherwise.
 */
int MAP_BlockRamOpen(BramPtr_s *pstBramPtr)
{
    int dev_fd;
    uint8_t *bram_map_base;

    if (NULL == pstBramPtr)
    {
        return -1;
    }

    dev_fd = open("/dev/mem", O_RDWR | O_NDELAY | O_DSYNC);
    if (0 > dev_fd)
    {
        fprintf(stderr, "%d, open /dev/mem failed: %s\n", __LINE__, strerror(errno));
        return -1;
    }

    printf("bram_map_base: 0x%lx, size: 0x%x\n", BRAM_BASE_ADDR, BRAM_MAX_SIZE);
    bram_map_base = (uint8_t *)mmap(NULL, BRAM_MAX_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, BRAM_BASE_ADDR);
    if (-1 == (unsigned long)bram_map_base)
    {
        close(dev_fd);
        fprintf(stderr, "%d, mmap failed: %s\n", __LINE__, strerror(errno));
        return -1;
    }

    pstBramPtr->fd = dev_fd;
    pstBramPtr->pucBase = bram_map_base;
    pstBramPtr->pstA53State = (A53State_s *)(bram_map_base + BRAM_A53_STATE_BASE_ADDR - BRAM_BASE_ADDR);
    pstBramPtr->pstA53Data = (A53Data_s *)(bram_map_base + BRAM_A53_DATA_BASE_ADDR - BRAM_BASE_ADDR);
    pstBramPtr->pstR5State = (R5State_s *)(bram_map_base + BRAM_R5_STATE_BASE_ADDR - BRAM_BASE_ADDR);
    pstBramPtr->pstR5Data = (R5Data_s *)(bram_map_base + BRAM_R5_DATA_BASE_ADDR - BRAM_BASE_ADDR);

    printf("%s %s exit!\n", __func__, MEM_MMAP_VERSION);
    return 0;
}

/*!
 * \brief block ram mmap close.
 *
 * \param[in] pstBramPtr W5 device connection's protocol type.
 * \return Status of function. 0 success, errors otherwise.
 */
int MAP_BlockRamClose(BramPtr_s *pstBramPtr)
{
    munmap(pstBramPtr->pucBase, BRAM_MAX_SIZE);
    close(pstBramPtr->fd);

    printf("%s %s exit!\n", __func__, MEM_MMAP_VERSION);
    return 0;
}

/*!
 * \brief PL state mmap open.
 *
 * \param[out] pstPlStatePtr PlStatePtr_s*, PL state pointer.
 * \return 0 on success, errors otherwise.
 */
int MAP_PlStateOpen(PlStatePtr_s *pstPlStatePtr)
{
    int dev_fd;
    uint8_t *map_base;

    if (NULL == pstPlStatePtr)
    {
        return -1;
    }

    dev_fd = open("/dev/mem", O_RDWR | O_NDELAY | O_DSYNC);
    if (0 > dev_fd)
    {
        fprintf(stderr, "%d, open /dev/mem failed: %s\n", __LINE__, strerror(errno));
        return -1;
    }

    printf("map base: 0x%lx, size: 0x%x\n", STATE_PL_ADDR, STATE_PL_SIZE);
    map_base = (uint8_t *)mmap(NULL, STATE_PL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, STATE_PL_ADDR);
    if (-1 == (unsigned long)map_base)
    {
        close(dev_fd);
        fprintf(stderr, "%d, mmap failed: %s\n", __LINE__, strerror(errno));
        return -1;
    }

    pstPlStatePtr->fd = dev_fd;
    pstPlStatePtr->pucBase = map_base;

    printf("%s %s exit!\n", __func__, MEM_MMAP_VERSION);
    return 0;
}

/*!
 * \brief PL state mmap close.
 *
 * \param[in] pstPlStatePtr PlStatePtr_s*, PL state pointer.
 * \return Status of function. 0 success, errors otherwise.
 */
int MAP_PlStateClose(PlStatePtr_s *pstPlStatePtr)
{
    munmap(pstPlStatePtr->pucBase, STATE_PL_SIZE);
    close(pstPlStatePtr->fd);

    printf("%s %s exit!\n", __func__, MEM_MMAP_VERSION);
    return 0;
}