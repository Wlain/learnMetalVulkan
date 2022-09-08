//
// Created by cwb on 2022/9/8.
//

#ifndef LEARNMETALVULKAN_HANDLEMTL_H
#define LEARNMETALVULKAN_HANDLEMTL_H
#include "device.h"
namespace backend
{
class DeviceMtl : public Device
{
public:
    explicit DeviceMtl(const Info& info);
    ~DeviceMtl() override;
    void init() override;
};
} // namespace backend

#endif // LEARNMETALVULKAN_HANDLEMTL_H
