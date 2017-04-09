/*
* Pedis is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* You may obtain a copy of the License at
*
*     http://www.gnu.org/licenses
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
*
*  Copyright (c) 2016-2026, Peng Jian, pstack@163.com. All rights reserved.
*
*/
#pragma once
#include "core/app-template.hh"
#include "core/future-util.hh"
#include "core/timer-set.hh"
#include "core/shared_ptr.hh"
#include "core/stream.hh"
#include "core/memory.hh"
#include "core/units.hh"
#include "core/distributed.hh"
#include "core/vector-data-sink.hh"
#include "core/bitops.hh"
#include "core/slab.hh"
#include "core/align.hh"
#include "net/api.hh"
#include "net/packet-data-source.hh"
#include "cache.hh"
namespace redis {
class reply_builder final {
public:
static future<> build(output_stream<char>& out, size_t size)
{
    scattered_message<char> msg;
    msg.append_static(msg_num_tag);
    msg.append(to_sstring(size));
    msg.append_static(msg_crlf);
    return out.write(std::move(msg));
}

template<bool Key, bool Value>
static future<> build(output_stream<char>& out, const cache_entry* e)
{
    if (e) {
        //build reply
        if (Key) {
            out.write(msg_batch_tag);
            out.write(to_sstring(e->key_size()));
            out.write(msg_crlf);
            out.write(e->key_data(), e->key_size());
            out.write(msg_crlf);
        }
        if (Value) {
            out.write(msg_batch_tag);
            if (e->type_of_integer()) {
               auto&& n = to_sstring(e->value_integer());
               out.write(to_sstring(n.size()));
               out.write(msg_crlf);
               out.write(n);
               out.write(msg_crlf);
            }
            else if (e->type_of_float()) {
               auto&& n = to_sstring(e->value_float());
               out.write(to_sstring(n.size()));
               out.write(msg_crlf);
               out.write(n);
               out.write(msg_crlf);
            }
            else if (e->type_of_bytes()) {
                out.write(to_sstring(e->value_bytes_size()));
                out.write(msg_crlf);
                out.write(e->value_bytes_data(), e->value_bytes_size());
                out.write(msg_crlf);
            }
            else {
               out.write(msg_type_err);
            }
        }
    }
    else {
        out.write(msg_not_found);
    }
    return make_ready_future<>();
}
};
}
