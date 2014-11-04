// renderTask.cpp
//
// RenderTask tells the renderer what to render. The task payload is sent over the network to individual hardware nodes, which render frames
// according to the parameters in the task payload. Render tasks are also used to direct nodes to do things like load world data and adjust overall
// rendering parameters. The taskType attribute determines what kind of task is being sent.

#include "renderTask.h"
#include <string.h>
#include <stdlib.h>

RenderTask::RenderTask(unsigned long seqNo) {
	this->seqNo = seqNo;
}

void RenderTask::setPayload(int taskType, size_t payloadDataSize, void *payloadData) {
	RenderTaskPayload *payload;

	payload = (RenderTaskPayload*)malloc(RENDERTASKPAYLOAD_HEADER_SIZE + payloadDataSize);
	if (payload) {
		payload->type = taskType;
		payload->totalSize = RENDERTASKPAYLOAD_HEADER_SIZE + payloadDataSize;
		payload->dataSize = payloadDataSize;
		memcpy(&payload->taskData, payloadData, payloadDataSize); // Load actual task data
	}
}

RenderTaskPayload *RenderTask::getPayload() {
	return this->payload;
}

void RenderTask::clearPayload() {
	if (payload != NULL) {
		free(payload);
		payload = NULL;
	}
}
