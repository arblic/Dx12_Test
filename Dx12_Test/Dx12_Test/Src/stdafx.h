#pragma once

#include <Windows.h>
#include <stdio.h>
#include <tchar.h>
#include <time.h>

#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <exception>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>

#define MY_IID_PPV_ARGS IID_PPV_ARGS

// jansson インクルード + リンク
#include "../../../mw/Jansson/include/jansson.h"
#pragma comment( lib, "jansson.lib" )

#include "Util/Utility.h"
#include "Util/FileProxy.h"

#include "D3D12/d3dx12.h"
#include "D3D12/dds.h"
#include "D3D12/DDSTextureLoader.h"
#include "D3D12/D3D12Device.h"
#include "D3D12/D3D12VertexBuffer.h"
#include "D3D12/D3D12IndexBuffer.h"
#include "D3D12/D3D12ConstantBuffer.h"
#include "D3D12/D3D12Shader.h"
#include "D3D12/D3D12Texture.h"
#include "D3D12/D3D12RootSignature.h"
