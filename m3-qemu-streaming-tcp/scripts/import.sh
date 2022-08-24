#!/usr/bin/env bash

set -e

MODALITY_AUTH_TOKEN="${MODALITY_AUTH_TOKEN:=$(<~/.config/modality_cli/.user_auth_token)}"

modality-trace-recorder-importer \
     --ingest-protocol-parent-url modality-ingest://127.0.0.1:14182 \
    --streaming \
    --startup-task-name 'm3' \
    --user-event-channel \
    --user-event-fmt-arg-attr-keys 'stats:%s %u %d %u %u:task,stack_size,stack_high_water,task_run_time,task_run_time_percentage' \
    --user-event-fmt-arg-attr-keys 'adc:%d:measurement' \
    --user-event-fmt-arg-attr-keys 'pwm:%d:value' \
    /tmp/trace.psf

exit 0
