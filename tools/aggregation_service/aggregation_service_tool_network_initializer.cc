// Copyright 2021 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tools/aggregation_service/aggregation_service_tool_network_initializer.h"

#include "base/check.h"
#include "content/public/browser/network_service_instance.h"
#include "mojo/core/embedder/embedder.h"
#include "mojo/public/cpp/bindings/remote.h"
#include "services/cert_verifier/public/mojom/cert_verifier_service_factory.mojom.h"
#include "services/network/public/cpp/weak_wrapper_shared_url_loader_factory.h"
#include "services/network/public/mojom/network_context.mojom.h"
#include "services/network/public/mojom/network_service.mojom.h"

namespace aggregation_service {

ToolNetworkInitializer::ToolNetworkInitializer() {
  // Initialize the network state as this tool runs independently from the
  // command line.
  mojo::core::Init();

  mojo::Remote<network::mojom::NetworkService> network_service_remote;
  network_service_ = network::NetworkService::Create(
      network_service_remote.BindNewPipeAndPassReceiver());

  auto network_context_params = network::mojom::NetworkContextParams::New();
  network_context_params->cert_verifier_params = content::GetCertVerifierParams(
      cert_verifier::mojom::CertVerifierCreationParams::New());

  mojo::Remote<network::mojom::NetworkContext> network_context_remote;
  network_context_ = std::make_unique<network::NetworkContext>(
      network_service_.get(),
      network_context_remote.BindNewPipeAndPassReceiver(),
      std::move(network_context_params));

  auto url_loader_factory_params =
      network::mojom::URLLoaderFactoryParams::New();
  url_loader_factory_params->process_id = network::mojom::kBrowserProcessId;
  url_loader_factory_params->is_corb_enabled = false;
  network_context_->CreateURLLoaderFactory(
      url_loader_factory_.BindNewPipeAndPassReceiver(),
      std::move(url_loader_factory_params));
  shared_url_loader_factory_ =
      base::MakeRefCounted<network::WeakWrapperSharedURLLoaderFactory>(
          url_loader_factory_.get());
}

ToolNetworkInitializer::~ToolNetworkInitializer() = default;

}  // namespace aggregation_service