#include "OpenLCBAliasRegistry.h"

// method definitions for the OpenLCBAliasRegistry

OpenLCBAliasRegistry::OpenLCBAliasRegistry(void){
	nodes = 0;
}

bool OpenLCBAliasRegistry::add(uint16_t alias, uint64_t nodeId, NodeAliasStatus status){
	if (nodes >= MAX_ALIASES)
	    return false;    // can't add - the table is full

	for(uint8_t i = 0; i < nodes; i++){
		if (alias == nodeEntry[i].alias || nodeId == nodeEntry[i].nodeId){
			return false;  // can't add the node - either the alias is in use or the node id is
			}
	}
	nodeEntry[nodes].alias = alias;
	nodeEntry[nodes].nodeId = nodeId;
	nodeEntry[nodes++].status = status;
	return true;
}

uint16_t OpenLCBAliasRegistry::getAlias(uint64_t nodeId){
	for(uint8_t i = 0; i < nodes; i++){
		if (nodeId == nodeEntry[i].nodeId){
		    return nodeEntry[i].alias;
		}
	}
	return 0; // not found
}
	
uint64_t OpenLCBAliasRegistry::getNodeId(uint16_t alias){
	uint8_t i = findAlias(alias);
	if (i < nodes){
		    return nodeEntry[i].nodeId;
		}
	return 0; // not found
	}
	
NodeAliasStatus OpenLCBAliasRegistry::getStatus(uint16_t alias){
	uint8_t i = findAlias(alias);
	if (i < nodes){
		return nodeEntry[i].status;
	}
	return nodeAliasNotFound;	// not found	

}	

bool OpenLCBAliasRegistry::JMRIRegistered(){
	bool found = false;
	for (uint8_t i = 0; i < nodes; i++){
		if (nodeEntry[i].status == RIDreceived ){
			if ((nodeEntry[i].nodeId & 0xFFFFFF0000000000ULL) == 0x0201120000000000ULL){
				found = true;
				break;
			}
		}
	return found;
	}
}

bool OpenLCBAliasRegistry::remove(uint16_t alias){
	uint8_t i = findAlias(alias);
	if (i < nodes){
		for (uint8_t j = i; j < nodes - 1; j++){ // move all successive entries down
			nodeEntry[i].alias = nodeEntry[i+1].alias;
			nodeEntry[i].nodeId = nodeEntry[i+1].nodeId;
			nodeEntry[i].status = nodeEntry[i+1].status;
		}
		nodes--; // one less node
		return true;
		}
	 
	 return false;  // didn't find an entry to update
}
	
bool OpenLCBAliasRegistry::setStatus(uint16_t alias, NodeAliasStatus status){
	uint8_t i = findAlias(alias);
	if (i < nodes){
		nodeEntry[i].status = status;
		return true;
		}
	 
	 return false;  // didn't find an entry to update
	}	

bool OpenLCBAliasRegistry::setNodeId(uint16_t alias, uint64_t newNodeId){
	uint8_t i = findAlias(alias);
	if (i < nodes){
		nodeEntry[i].nodeId = newNodeId;
		return true;
		}
	 
	 return false;  // didn't find an entry to update
	}		

uint8_t OpenLCBAliasRegistry::findAlias(uint16_t alias){
    for (uint8_t i = 0; i < nodes; i++){
		if (nodeEntry[i].alias == alias){
			return i;
		} else {
			return MAX_ALIASES + 1;  // out of range value; not found
		}
	}
    
}

