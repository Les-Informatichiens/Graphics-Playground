//
// Created by Jonathan Richard on 2024-02-04.
//

#pragma once

#include <spirv_glsl.hpp>

enum class DataType {
    None,
    Bool,
    Int,
    UInt,
    Float,
    Double,
    BVec2,
    BVec3,
    BVec4,
    IVec2,
    IVec3,
    IVec4,
    UVec2,
    UVec3,
    UVec4,
    Vec2,
    Vec3,
    Vec4,
    DVec2,
    DVec3,
    DVec4,
    Mat2,
    Mat2x3,
    Mat2x4,
    Mat3x2,
    Mat3,
    Mat3x4,
    Mat4x2,
    Mat4x3,
    Mat4,
    DMat2,
    DMat2x3,
    DMat2x4,
    DMat3x2,
    DMat3,
    DMat3x4,
    DMat4x2,
    DMat4x3,
    DMat4
};

DataType SPIRTypeToDataType(const spirv_cross::SPIRType& type) {
    switch (type.basetype) {
        case spirv_cross::SPIRType::Boolean:
            switch (type.vecsize) {
                case 1:                        return DataType::Bool;
                case 2:                        return DataType::BVec2;
                case 3:                        return DataType::BVec3;
                case 4:                        return DataType::BVec4;
            }
        case spirv_cross::SPIRType::Int:
            switch (type.vecsize) {
                case 1:                        return DataType::Int;
                case 2:                        return DataType::IVec2;
                case 3:                        return DataType::IVec3;
                case 4:                        return DataType::IVec4;
            }
        case spirv_cross::SPIRType::UInt:
        case 1:
            switch (type.vecsize) {
                case 1:                        return DataType::UInt;
                case 2:                        return DataType::UVec2;
                case 3:                        return DataType::UVec3;
                case 4:                        return DataType::UVec4;
            }
        case spirv_cross::SPIRType::Float:
            switch (type.columns) {
                case 1:
                    switch (type.vecsize) {
                        case 1:                  return DataType::Float;
                        case 2:                  return DataType::Vec2;
                        case 3:                  return DataType::Vec3;
                        case 4:                  return DataType::Vec4;
                    }
                case 2:
                    switch (type.vecsize) {
                        case 2:                  return DataType::Mat2;
                        case 3:                  return DataType::Mat2x3;
                        case 4:                  return DataType::Mat2x4;
                    }
                case 3:
                    switch (type.vecsize) {
                        case 2:                  return DataType::Mat3x2;
                        case 3:                  return DataType::Mat3;
                        case 4:                  return DataType::Mat3x4;
                    }
                case 4:
                    switch (type.vecsize) {
                        case 2:                  return DataType::Mat4x2;
                        case 3:                  return DataType::Mat3x4;
                        case 4:                  return DataType::Mat4;
                    }
            }
        case spirv_cross::SPIRType::Double:
            switch (type.columns) {
                case 1:
                    switch (type.vecsize) {
                        case 1:                  return DataType::Double;
                        case 2:                  return DataType::DVec2;
                        case 3:                  return DataType::DVec3;
                        case 4:                  return DataType::DVec4;
                    }
                case 2:
                    switch (type.vecsize) {
                        case 2:                  return DataType::DMat2;
                        case 3:                  return DataType::DMat2x3;
                        case 4:                  return DataType::DMat2x4;
                    }
                case 3:
                    switch (type.vecsize) {
                        case 2:                  return DataType::DMat3x2;
                        case 3:                  return DataType::DMat3;
                        case 4:                  return DataType::DMat3x4;
                    }
                case 4:
                    switch (type.vecsize) {
                        case 2:                  return DataType::DMat4x2;
                        case 3:                  return DataType::DMat3x4;
                        case 4:                  return DataType::DMat4;
                    }
            }
    }
    return DataType::None;
}

