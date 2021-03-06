#ifndef MAXPOOLINGDERIVATIVE_H
#define MAXPOOLINGDERIVATIVE_H

#include "Operator.h"
#include <cudnn.h>

namespace FreeWill
{

    template<DeviceType DeviceUsed = CPU, typename DataType = float>
    class MaxPoolingDerivative : public Operator<DeviceUsed>
    {
    protected:
        using Operator<DeviceUsed>::input;
        using Operator<DeviceUsed>::output;
        cudnnPoolingDescriptor_t m_poolingDescriptor;
        cudnnTensorDescriptor_t m_outputGPUTensorDescriptor;
        cudnnTensorDescriptor_t m_outputDeltaGPUTensorDescriptor;
        cudnnTensorDescriptor_t m_inputGPUTensorDescriptor;
        cudnnTensorDescriptor_t m_inputDeltaGPUTensorDescriptor;


    public:
        MaxPoolingDerivative()
            :Operator<DeviceUsed>({"Output","OutputGrad","Input", "SwitchX", "SwitchY"},{"InputGrad"}),
            m_poolingDescriptor(0),
            m_outputGPUTensorDescriptor(0),
            m_outputDeltaGPUTensorDescriptor(0),
            m_inputGPUTensorDescriptor(0),
            m_inputDeltaGPUTensorDescriptor(0)
        {
            if constexpr ((DeviceUsed & (GPU | GPU_CUDA)) != 0)
            {
                RUN_CUDNN(cudnnCreatePoolingDescriptor(&m_poolingDescriptor));
                RUN_CUDNN(cudnnCreateTensorDescriptor(&m_outputGPUTensorDescriptor));
                RUN_CUDNN(cudnnCreateTensorDescriptor(&m_outputDeltaGPUTensorDescriptor));
                RUN_CUDNN(cudnnCreateTensorDescriptor(&m_inputGPUTensorDescriptor));
                RUN_CUDNN(cudnnCreateTensorDescriptor(&m_inputDeltaGPUTensorDescriptor));
            }
        }

        ~MaxPoolingDerivative()
        {
            if constexpr ((DeviceUsed & (GPU | GPU_CUDA)) != 0)
            {
                RUN_CUDNN(cudnnDestroyPoolingDescriptor(m_poolingDescriptor));
                RUN_CUDNN(cudnnDestroyTensorDescriptor(m_outputGPUTensorDescriptor));
                RUN_CUDNN(cudnnDestroyTensorDescriptor(m_outputDeltaGPUTensorDescriptor));
                RUN_CUDNN(cudnnDestroyTensorDescriptor(m_inputGPUTensorDescriptor));
                RUN_CUDNN(cudnnDestroyTensorDescriptor(m_inputDeltaGPUTensorDescriptor));

                m_poolingDescriptor = 0;
                m_outputGPUTensorDescriptor = 0;
                m_outputDeltaGPUTensorDescriptor = 0;
                m_inputGPUTensorDescriptor = 0;
                m_inputDeltaGPUTensorDescriptor = 0;
            }
        }

        virtual bool init() override
        {
            FAIL_IF (!input("OutputGrad") || !output("InputGrad"));

            if constexpr ((DeviceUsed & (CPU | CPU_NAIVE)) != 0)
            {
                FAIL_IF(!input("SwitchX") || !input("SwitchY"));
                FAIL_IF (input("OutputGrad")->shape() != input("SwitchX")->shape());
                FAIL_IF (input("OutputGrad")->shape() != input("SwitchY")->shape());
            }
            else if constexpr ((DeviceUsed & (GPU | GPU_CUDA)) !=0)
            {
                FAIL_IF(!input("Output") || !input("Input"));
            }

            FAIL_IF (output("InputGrad")->shape()[0] != input("OutputGrad")->shape()[0]);

            FAIL_IF (output("InputGrad")->shape()[1] != input("OutputGrad")->shape()[1] * 2);

            FAIL_IF (output("InputGrad")->shape()[2] != input("OutputGrad")->shape()[2]*2);

            FAIL_IF (output("InputGrad")->shape()[3] != input("OutputGrad")->shape()[3]);
            
            if constexpr ((DeviceUsed & (GPU | GPU_CUDA)) != 0)
            {
                cudnnDataType_t dataType = CUDNN_DATA_FLOAT;
                if constexpr (std::is_same<DataType,float>::value)
                {
                    dataType = CUDNN_DATA_FLOAT;
                }
                else if constexpr (std::is_same<DataType,double>::value)
                {
                    dataType = CUDNN_DATA_DOUBLE;
                }

                unsigned int batchSize = input("Input")->shape()[3];
                unsigned int channelSize = input("Input")->shape()[0];
                unsigned int width = input("Input")->shape()[1];
                unsigned int height = input("Input")->shape()[2];
       
                RUN_CUDNN(cudnnSetPooling2dDescriptor( m_poolingDescriptor,
                                                       CUDNN_POOLING_MAX,
                                                       CUDNN_NOT_PROPAGATE_NAN,
                                                       2,
                                                       2,
                                                       0,
                                                       0,
                                                       2,
                                                       2));

                RUN_CUDNN(cudnnSetTensor4dDescriptor( m_inputGPUTensorDescriptor,
                                                      CUDNN_TENSOR_NHWC,
                                                      dataType,
                                                      batchSize,
                                                      channelSize,
                                                      height, width));

                RUN_CUDNN(cudnnSetTensor4dDescriptor(m_outputGPUTensorDescriptor,
                                                     CUDNN_TENSOR_NHWC,
                                                     dataType,
                                                     batchSize,
                                                     channelSize,
                                                     height/2, width/2));
 
                RUN_CUDNN(cudnnSetTensor4dDescriptor( m_inputDeltaGPUTensorDescriptor,
                                                      CUDNN_TENSOR_NHWC,
                                                      dataType,
                                                      batchSize,
                                                      channelSize,
                                                      height, width));

                RUN_CUDNN(cudnnSetTensor4dDescriptor(m_outputDeltaGPUTensorDescriptor,
                                                     CUDNN_TENSOR_NHWC,
                                                     dataType,
                                                     batchSize,
                                                     channelSize,
                                                     height/2, width/2));
                         
            }

            return true;
        }

        virtual void evaluate() override
        {
            Tensor<DeviceUsed, DataType> *_inputGrad = (Tensor<DeviceUsed, DataType> *) output("InputGrad");
            Tensor<DeviceUsed, DataType> *_outputGrad = (Tensor<DeviceUsed, DataType> *) input("OutputGrad");


            unsigned int outputWidth = _outputGrad->shape()[1];
            unsigned int outputHeight = _outputGrad->shape()[2];
            unsigned int batchSize = _outputGrad->shape()[3];
            unsigned int depthSize = _outputGrad->shape()[0];

            if constexpr ((DeviceUsed & (CPU | CPU_NAIVE)) != 0)
            {
                Tensor<DeviceUsed, unsigned int> *_switchX = (Tensor<DeviceUsed, unsigned int>*) input("SwitchX");
                Tensor<DeviceUsed, unsigned int> *_switchY = (Tensor<DeviceUsed, unsigned int> *) input("SwitchY");


                for(unsigned int b = 0;b<batchSize;++b)
                {
                    for(unsigned int y = 0; y<outputHeight;++y)
                    {
                        for(unsigned int x = 0;x<outputWidth;++x)
                        {
                            for(unsigned int depth = 0;depth<depthSize;++depth)
                            {
                                unsigned int index = (b*outputWidth*outputHeight + y*outputWidth +x)*depthSize + depth;
                                unsigned int inputX = (*_switchX)[index];
                                unsigned int inputY = (*_switchY)[index];

                                (*_inputGrad)[(b*outputWidth*outputHeight*4 + inputY*outputWidth*2 + inputX)*depthSize + depth] = 
                                    (*_outputGrad)[index];
                            }
                        }
                    }
                }
            }
            else if constexpr ((DeviceUsed & (GPU | GPU_CUDA))!=0)
            {
                Tensor<DeviceUsed, DataType> *_input = (Tensor<DeviceUsed, DataType> *) input("Input");
                Tensor<DeviceUsed, DataType> *_output = (Tensor<DeviceUsed, DataType> *) input("Output");

                DataType alpha = 1.0;
                DataType beta = 0.0;

                RUN_CUDNN(cudnnPoolingBackward( Context<DeviceUsed>::getSingleton().cudnnHandle(),
                                                m_poolingDescriptor,
                                                &alpha,
                                                m_outputGPUTensorDescriptor,
                                                _output->gpuDataHandle(),
                                                m_outputGPUTensorDescriptor,
                                                _outputGrad->gpuDataHandle(),
                                                m_inputGPUTensorDescriptor,
                                                _input->gpuDataHandle(),
                                                &beta,
                                                m_inputDeltaGPUTensorDescriptor,
                                                _inputGrad->gpuDataHandle()));            
            }
        }
    };
}

#endif
