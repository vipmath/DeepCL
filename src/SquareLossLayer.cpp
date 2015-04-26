// Copyright Hugh Perkins 2014 hughperkins at gmail
//
// This Source Code Form is subject to the terms of the Mozilla Public License, 
// v. 2.0. If a copy of the MPL was not distributed with this file, You can 
// obtain one at http://mozilla.org/MPL/2.0/.

#include "SquareLossLayer.h"
#include "LossLayer.h"
#include "LayerMaker.h"

using namespace std;

#undef VIRTUAL
#undef STATIC
#define VIRTUAL
#define STATIC

SquareLossLayer::SquareLossLayer( Layer *previousLayer, SquareLossMaker *maker ) :
        LossLayer( previousLayer, maker ),
        errors( 0 ),
        allocatedSize( 0 ) {
}
VIRTUAL SquareLossLayer::~SquareLossLayer(){
    if( errors != 0 ) {
        delete[] errors;
    }
}
VIRTUAL std::string SquareLossLayer::getClassName() const {
    return "SquareLossLayer";
}
VIRTUAL float*SquareLossLayer::getGradInput() {
    return errors;
}
//VIRTUAL float*SquareLossLayer::getDerivLossBySumForUpstream() {
//    return gradOutput;
//}
VIRTUAL float SquareLossLayer::calcLoss( float const *expected ) {
    float loss = 0;
    float *output = getOutput();
//    cout << "SquareLossLayer::calcLoss" << endl;
    // this is matrix subtraction, then element-wise square, then aggregation
    int numPlanes = previousLayer->getOutputPlanes();
    int imageSize = previousLayer->getOutputImageSize();
    for( int imageId = 0; imageId < batchSize; imageId++ ) {
        for( int plane = 0; plane < numPlanes; plane++ ) {
            for( int outRow = 0; outRow < imageSize; outRow++ ) {
                for( int outCol = 0; outCol < imageSize; outCol++ ) {
                    int resultOffset = ( ( imageId
                         * numPlanes + plane )
                         * imageSize + outRow )
                         * imageSize + outCol;
 //                   int resultOffset = getResultIndex( imageId, plane, outRow, outCol ); //imageId * numPlanes + out;
                    float expectedOutput = expected[resultOffset];
                    float actualOutput = output[resultOffset];
                    float diff = actualOutput - expectedOutput;
                    float squarederror = diff * diff;
                    loss += squarederror;
                }
            }
        }            
    }
    loss *= 0.5f;
//    cout << "loss " << loss << endl;
    return loss;
 }
VIRTUAL void SquareLossLayer::setBatchSize( int batchSize ) {
    if( batchSize <= allocatedSize ) {
        this->batchSize = batchSize;
        return;
    }
    if( errors != 0 ) {
        delete[] errors;
    }
    this->batchSize = batchSize;
    allocatedSize = batchSize;
    errors = new float[ batchSize * previousLayer->getOutputSize() ];
}
VIRTUAL void SquareLossLayer::calcErrors( float const*expectedOutput ) {
    ActivationFunction const*fn = previousLayer->getActivationFunction();
    int outputSize = previousLayer->getOutputSize();
    float *output = previousLayer->getOutput();
    for( int i = 0; i < outputSize; i++ ) {
        float result = output[i];
        float partialOutBySum = fn->calcDerivative( result );
        float partialLossByOut = result - expectedOutput[i];
        errors[i] = partialLossByOut * partialOutBySum;
    }
}
VIRTUAL int SquareLossLayer::getPersistSize() const {
    return 0;
}
VIRTUAL std::string SquareLossLayer::asString() const {
    return "SquareLossLayer{}";
}

