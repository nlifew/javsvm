//
// Created by edz on 2022/1/31.
//

#ifndef JAVSVM_SAFETY_POINT_H
#define JAVSVM_SAFETY_POINT_H

namespace javsvm
{


extern char *safety_point_trap;
#define SAFETY_POINT safety_point_trap[0] = 0;


}; // namespace javsvm


#endif //JAVSVM_SAFETY_POINT_H
