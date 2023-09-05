#pragma once

#include <array>
#include "random.h"
#include <opentelemetry/trace/trace_id.h>
#include <opentelemetry/trace/span_id.h>
#include <opentelemetry/trace/propagation/http_trace_context.h>
#include <opentelemetry/sdk/trace/random_id_generator.h>

#include "str_view.hpp"

class AWSXRayIDGenerator : public opentelemetry::sdk::trace::IdGenerator
{
public:
  opentelemetry::trace::TraceId GenerateTraceId() noexcept override {
    {
      uint8_t trace_id_buf[opentelemetry::trace::TraceId::kSize];

      const auto p1 = std::chrono::system_clock::now();
      uint64_t ts = std::chrono::duration_cast<std::chrono::seconds>(p1.time_since_epoch()).count();
      uint64_t lo_rand = Random::GenerateRandom64();
      uint64_t hi_rand = Random::GenerateRandom64() & 0x00000000FFFFFFFF;
      uint64_t hi = ts << 32 | hi_rand;
      memcpy(&trace_id_buf[0], &lo_rand, sizeof(uint64_t));
      memcpy(&trace_id_buf[sizeof(uint64_t)], &hi, sizeof(uint64_t));
      for(size_t l=0,h=opentelemetry::trace::TraceId::kSize-1; l < h; l++, h--)
      {
        std::swap(trace_id_buf[l], trace_id_buf[h]);
      }
      return opentelemetry::trace::TraceId(trace_id_buf);
    }
  }

  opentelemetry::trace::SpanId GenerateSpanId() noexcept override {
    {
      uint8_t span_id_buf[opentelemetry::trace::SpanId::kSize];

      Random::GenerateRandomBuffer(span_id_buf);
      return opentelemetry::trace::SpanId(span_id_buf);
    }
  }
};

struct TraceContext {
    opentelemetry::trace::TraceId traceId;
    opentelemetry::trace::SpanId spanId;
    bool sampled;
    StrView state;

    static const auto Size =
        opentelemetry::trace::propagation::kTraceParentSize;

    static TraceContext generate(bool sampled, TraceContext parent = {})
    {
        // opentelemetry::sdk::trace::RandomIdGenerator idGen;
	AWSXRayIDGenerator idGen;

        return {parent.traceId.IsValid() ?
                    parent.traceId : idGen.GenerateTraceId(),
                idGen.GenerateSpanId(),
                sampled,
                parent.state};
    }

    static TraceContext parse(StrView trace, StrView state)
    {
        using namespace opentelemetry::trace::propagation;

        std::array<StrView, 4> parts;
        if (detail::SplitString(trace, '-', parts.data(), 4) != 4) {
            return TraceContext{};
        }

        auto version = parts[0];
        auto traceId = parts[1];
        auto spanId = parts[2];
        auto flags = parts[3];

        if (version != "00") {
            return TraceContext{};
        }

        if (traceId.size() != kTraceIdSize || spanId.size() != kSpanIdSize ||
            flags.size() != kTraceFlagsSize)
        {
            return TraceContext{};
        }

        if (!detail::IsValidHex(traceId) || !detail::IsValidHex(spanId) ||
            !detail::IsValidHex(flags))
        {
            return TraceContext{};
        }

        return {HttpTraceContext::TraceIdFromHex(traceId),
                HttpTraceContext::SpanIdFromHex(spanId),
                HttpTraceContext::TraceFlagsFromHex(flags).IsSampled(),
                state};
    }

    static void serialize(const TraceContext& tc, char* out)
    {
        using namespace opentelemetry::trace::propagation;

        *out++ = '0';
        *out++ = '0';
        *out++ = '-';

        tc.traceId.ToLowerBase16({out, kTraceIdSize});
        out += kTraceIdSize;
        *out++ = '-';

        tc.spanId.ToLowerBase16({out, kSpanIdSize});
        out += kSpanIdSize;
        *out++ = '-';

        *out++ = '0';
        *out++ = tc.sampled ? '1' : '0';
    }
};
