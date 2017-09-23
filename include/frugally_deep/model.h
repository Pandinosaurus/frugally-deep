// Copyright 2016, Tobias Hermann.
// https://github.com/Dobiasd/frugally-deep
// Distributed under the MIT License.
// (See accompanying LICENSE file or at
//  https://opensource.org/licenses/MIT)

#pragma once

#include "frugally_deep/typedefs.h"

#include "frugally_deep/tensor3.h"

#include "frugally_deep/layers/layer.h"

#include <cstddef>
#include <memory>

namespace fd
{

class model : public layer
{
public:

    explicit model(const std::string& name,
        const layer_ptrs& layers,
        const node_connections& input_connections,
        const node_connections& output_connections)
            : layer(name),
            layers_(layers),
            input_connections_(input_connections),
            output_connections_(output_connections)
    {
        const auto get_layer_name = [](const layer_ptr& l_ptr) -> std::string
        {
            return l_ptr->name_;
        };
        assertion(fplus::all_unique_on(get_layer_name, layers),
            "layer names must be unique");
    }
    virtual ~model()
    {
    }
    tensor3s predict(const tensor3s& inputs) const
    {
        return apply_impl(inputs);
    }

    tensor3 get_output(const layer_ptrs& layers,
        output_dict& output_cache,
        std::size_t node_idx, std::size_t tensor_idx) const override
    {
        // https://stackoverflow.com/questions/46011749/understanding-keras-model-architecture-node-index-of-nested-model
        node_idx = node_idx - 1;
        assertion(node_idx < nodes_.size(), "invalid node index");
        return layer::get_output(layers, output_cache, node_idx, tensor_idx);
    }

protected:
    virtual tensor3s apply_impl(const tensor3s& inputs) const override
    {
        output_dict output_cache;

        assertion(inputs.size() == input_connections_.size(),
            "invalid number of input tensors for this model: " +
            fplus::show(input_connections_.size()) + " required but " +
            fplus::show(inputs.size()) + " provided");

        for (std::size_t i = 0; i < inputs.size(); ++i)
        {
            output_cache[input_connections_[i].without_tensor_idx()] =
                {inputs[i]};
        }

        const auto get_output = [this, &output_cache]
            (const node_connection& conn) -> tensor3
        {
            return get_layer(layers_, conn.layer_id_)->get_output(
                layers_, output_cache, conn.node_idx_, conn.tensor_idx_);
        };
        return fplus::transform(get_output, output_connections_);
    }
    layer_ptrs layers_;
    node_connections input_connections_;
    node_connections output_connections_;
};

} // namespace fd
