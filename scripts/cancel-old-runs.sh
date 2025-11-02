#!/bin/bash
# Script to cancel old or stuck GitHub Actions workflow runs

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "=== GitHub Actions Run Cancellation Tool ==="
echo ""

# Function to cancel a run by ID
cancel_run() {
    local run_id=$1
    echo -e "${YELLOW}Cancelling run ID: $run_id${NC}"
    if gh run cancel "$run_id"; then
        echo -e "${GREEN}✓ Successfully cancelled run $run_id${NC}"
    else
        echo -e "${RED}✗ Failed to cancel run $run_id${NC}"
    fi
}

# Function to list active runs
list_active_runs() {
    echo -e "${YELLOW}=== Active Runs (not completed) ===${NC}"
    gh run list --limit 100 --json databaseId,workflowName,status,conclusion,createdAt,headBranch,url \
        --jq '.[] | select(.status != "completed") | 
              "\(.databaseId)\t\(.workflowName)\t\(.status)\t\(.conclusion // "N/A")\t\(.headBranch)\t\(.createdAt)"' | \
        column -t -s $'\t'
    echo ""
}

# Function to cancel runs by workflow name
cancel_workflow_runs() {
    local workflow_name=$1
    echo -e "${YELLOW}=== Cancelling runs for workflow: $workflow_name ===${NC}"
    local run_ids=$(gh run list --workflow "$workflow_name" --limit 50 --json databaseId,status \
        --jq '.[] | select(.status != "completed") | .databaseId')
    
    if [ -z "$run_ids" ]; then
        echo -e "${GREEN}No active runs found for workflow: $workflow_name${NC}"
        return
    fi
    
    for run_id in $run_ids; do
        cancel_run "$run_id"
    done
}

# Function to cancel runs older than N days
cancel_old_runs() {
    local days=$1
    echo -e "${YELLOW}=== Cancelling runs older than $days days ===${NC}"
    local cutoff=$(date -u -d "$days days ago" +%Y-%m-%dT%H:%M:%SZ 2>/dev/null || date -u -v-${days}d +%Y-%m-%dT%H:%M:%SZ 2>/dev/null || echo "")
    
    if [ -z "$cutoff" ]; then
        echo -e "${RED}Error: Could not calculate cutoff date${NC}"
        return
    fi
    
    local run_ids=$(gh run list --limit 200 --json databaseId,workflowName,status,createdAt \
        --jq ".[] | select(.status != \"completed\" and .createdAt < \"$cutoff\") | .databaseId")
    
    if [ -z "$run_ids" ]; then
        echo -e "${GREEN}No old active runs found${NC}"
        return
    fi
    
    for run_id in $run_ids; do
        cancel_run "$run_id"
    done
}

# Main menu
if [ "$1" = "--list" ] || [ "$1" = "-l" ]; then
    list_active_runs
elif [ "$1" = "--workflow" ] || [ "$1" = "-w" ]; then
    if [ -z "$2" ]; then
        echo -e "${RED}Error: Workflow name required${NC}"
        echo "Usage: $0 --workflow <workflow-name>"
        exit 1
    fi
    cancel_workflow_runs "$2"
elif [ "$1" = "--old" ] || [ "$1" = "-o" ]; then
    days=${2:-7}  # Default to 7 days
    cancel_old_runs "$days"
elif [ "$1" = "--cancel" ] || [ "$1" = "-c" ]; then
    if [ -z "$2" ]; then
        echo -e "${RED}Error: Run ID required${NC}"
        echo "Usage: $0 --cancel <run-id>"
        exit 1
    fi
    cancel_run "$2"
else
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  --list, -l                    List all active (non-completed) runs"
    echo "  --workflow, -w <name>         Cancel all active runs for a specific workflow"
    echo "  --old, -o [days]              Cancel active runs older than N days (default: 7)"
    echo "  --cancel, -c <run-id>         Cancel a specific run by ID"
    echo ""
    echo "Examples:"
    echo "  $0 --list"
    echo "  $0 --workflow cross-platform-builds.yml"
    echo "  $0 --old 3"
    echo "  $0 --cancel 123456789"
    echo ""
    
    # Show active runs by default
    list_active_runs
fi

