# 
# Zillians MMO
# Copyright (C) 2007-2012 Zillians.com, Inc.
# For more information see http:#www.zillians.com
#
# Zillians MMO is the library and runtime for massive multiplayer online game
# development in utility computing model, which runs as a service for every 
# developer to build their virtual world running on our GPU-assisted machines
#
# This is a close source library intended to be used solely within Zillians.com
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
#
# Contact Information: info@zillians.com
#

ADD_SUBDIRECTORY(libzillians-common-core)

IF(COROSYNC_FOUND)
	ADD_SUBDIRECTORY(libzillians-common-networking)
ENDIF(COROSYNC_FOUND)

ADD_SUBDIRECTORY(libzillians-common-networking-tcp)

IF(RDMA_FOUND)
    ADD_SUBDIRECTORY(libzillians-common-networking-infiniband)
ENDIF(RDMA_FOUND)

IF(COROSYNC_FOUND)
    ADD_SUBDIRECTORY(libzillians-common-networking-cpg)
ENDIF(COROSYNC_FOUND)

ADD_SUBDIRECTORY(libzillians-common-utility)
