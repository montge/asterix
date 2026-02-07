# Git Worktree Workflow for OpenSpec Changes

This guide explains how to use git worktrees to work on multiple OpenSpec changes simultaneously, maintaining clean separation between different features and specs.

---

## What are Git Worktrees?

Git worktrees allow you to have multiple working directories for the same repository, each checked out to a different branch. This is perfect for OpenSpec workflows where you might be:
- Working on multiple changes in parallel
- Reviewing one change while developing another
- Maintaining different experimental branches

**Benefits:**
- No context switching (no `git stash` or branch switching)
- Work on multiple features simultaneously
- Clean separation of changes
- Faster testing across branches

---

## Setup

### 1. Initialize Main Repository

Your main repository is already set up. This will be the "bare" worktree where you primarily work.

```bash
# Current location
cd /home/user/asterix
```

### 2. Create Worktrees for OpenSpec Changes

When starting a new OpenSpec change, create a dedicated worktree:

```bash
# Create a new worktree for an OpenSpec change
git worktree add ../asterix-feature/<change-name> -b feature/<change-name>

# Example: Creating a change for "add-go-bindings"
git worktree add ../asterix-feature/add-go-bindings -b feature/add-go-bindings
```

This creates:
- A new branch `feature/add-go-bindings`
- A new working directory at `../asterix-feature/add-go-bindings`
- The new directory is fully independent (separate git state)

---

## OpenSpec + Worktree Workflow

### Starting a New Change

```bash
# 1. Create worktree for the change
git worktree add ../asterix-feature/wireshark-phase3 -b feature/wireshark-phase3

# 2. Navigate to the new worktree
cd ../asterix-feature/wireshark-phase3

# 3. Start OpenSpec change
# In Claude Code, use: /opsx:new wireshark-phase3

# 4. Work through the OpenSpec workflow
# /opsx:ff       - Generate all planning docs
# /opsx:apply    - Implement tasks
# /opsx:archive  - Archive when complete

# 5. Commit and push from the worktree
git add .
git commit -m "feat: Complete Wireshark Phase 3

Relates to #22

- Implemented multi-category support
- Added color coding
- Integrated statistics

/opsx:archive wireshark-phase3"
git push -u origin feature/wireshark-phase3
```

### Working on Multiple Changes Simultaneously

```bash
# Main repo - reviewing PRs
cd /home/user/asterix

# Worktree 1 - implementing feature A
cd ../asterix-feature/add-go-bindings
# /opsx:apply

# Worktree 2 - implementing feature B
cd ../asterix-feature/improve-performance
# /opsx:apply

# Worktree 3 - fixing bug
cd ../asterix-feature/fix-memory-leak
# /opsx:apply
```

Each worktree is completely independent - you can build, test, and commit without affecting the others.

---

## Directory Structure Example

```
/home/user/
├── asterix/                          # Main repository
│   ├── .git/                         # Git repository
│   ├── .claude/                      # Claude Code skills (gitignored)
│   ├── openspec/                     # OpenSpec configuration
│   │   ├── config.yaml
│   │   ├── changes/
│   │   └── specs/
│   └── src/                          # Source code
│
├── asterix-feature/                  # Worktrees directory
│   ├── add-go-bindings/              # Worktree 1
│   │   ├── .git                      # -> /home/user/asterix/.git
│   │   ├── openspec/changes/add-go-bindings/
│   │   └── go-bindings/              # New Go bindings code
│   │
│   ├── wireshark-phase3/             # Worktree 2
│   │   ├── .git                      # -> /home/user/asterix/.git
│   │   ├── openspec/changes/wireshark-phase3/
│   │   └── plugins/epan/asterix/     # Modified Wireshark plugin
│   │
│   └── improve-performance/          # Worktree 3
│       ├── .git                      # -> /home/user/asterix/.git
│       ├── openspec/changes/improve-performance/
│       └── src/asterix/              # Optimized core
```

---

## Common Workflows

### Scenario 1: Code Review While Developing

```bash
# You're developing a feature in a worktree
cd ../asterix-feature/add-go-bindings
# /opsx:apply - implementing tasks

# PR comes in for review - switch to main repo
cd /home/user/asterix
git fetch origin
git checkout pr-branch
# Review the PR

# Go back to your feature work instantly
cd ../asterix-feature/add-go-bindings
# Continue where you left off (no stashing needed)
```

### Scenario 2: Testing Across Branches

```bash
# Test feature A
cd ../asterix-feature/add-go-bindings
cmake -B build && cmake --build build
./install/test/test.sh

# Test feature B without affecting feature A
cd ../asterix-feature/wireshark-phase3
cmake -B build-wireshark && cmake --build build-wireshark
./install/test/test.sh

# Compare results side-by-side
```

### Scenario 3: Merging Multiple Features

```bash
# Main repo - create integration branch
cd /home/user/asterix
git checkout -b integrate/release-v2.9.0

# Merge feature A
git merge --no-ff feature/add-go-bindings

# Merge feature B
git merge --no-ff feature/wireshark-phase3

# Test integration
cmake -B build && cmake --build build
./install/test/test.sh

# If conflicts or issues, features are still intact in their worktrees
```

---

## Worktree Management Commands

### List All Worktrees

```bash
git worktree list

# Example output:
# /home/user/asterix                               a2fe360 [claude/check-public-visibility-01Noa9JRQ6SVP8EeWXcWdDQG]
# /home/user/asterix-feature/add-go-bindings       abc1234 [feature/add-go-bindings]
# /home/user/asterix-feature/wireshark-phase3      def5678 [feature/wireshark-phase3]
```

### Remove a Worktree

```bash
# When done with a feature (already merged and pushed)
git worktree remove ../asterix-feature/add-go-bindings

# Or if you need to force remove
git worktree remove --force ../asterix-feature/add-go-bindings

# Clean up the branch
git branch -d feature/add-go-bindings
```

### Move a Worktree

```bash
# If you need to relocate a worktree
git worktree move ../asterix-feature/old-location ../asterix-feature/new-location
```

### Prune Stale Worktrees

```bash
# If a worktree directory was manually deleted, clean up the reference
git worktree prune
```

---

## OpenSpec-Specific Tips

### Tip 1: One Worktree Per OpenSpec Change

```bash
# Create worktree when starting a new OpenSpec change
/opsx:new add-nodejs-async-api

# In terminal:
git worktree add ../asterix-feature/add-nodejs-async-api -b feature/add-nodejs-async-api
cd ../asterix-feature/add-nodejs-async-api

# Now use OpenSpec workflow in the worktree
# /opsx:ff
# /opsx:apply
```

### Tip 2: Archive in Worktree, Merge in Main

```bash
# Complete the change in worktree
cd ../asterix-feature/add-nodejs-async-api
# /opsx:archive add-nodejs-async-api

# Commit and push
git add openspec/changes/archive/2026-02-07-add-nodejs-async-api/
git commit -m "feat: Add Node.js async API"
git push -u origin feature/add-nodejs-async-api

# Switch to main repo to create PR
cd /home/user/asterix
gh pr create --base main --head feature/add-nodejs-async-api
```

### Tip 3: Sync Config Across Worktrees

Changes to `openspec/config.yaml` in the main repo are visible in all worktrees (shared `.git`):

```bash
# Update config in main repo
cd /home/user/asterix
vim openspec/config.yaml
git add openspec/config.yaml
git commit -m "chore: Update OpenSpec context"

# Pull changes in worktrees
cd ../asterix-feature/add-go-bindings
git pull origin main
```

---

## Best Practices

### 1. Consistent Naming

```bash
# Branch naming
feature/<openspec-change-name>
bugfix/<issue-number>-<short-desc>
hotfix/<version>-<issue>

# Worktree directory naming (match branch name)
../asterix-feature/<openspec-change-name>
../asterix-bugfix/<issue-number>-<short-desc>
../asterix-hotfix/<version>-<issue>
```

### 2. Keep Worktrees Clean

```bash
# Regularly clean up completed worktrees
git worktree list
git worktree remove ../asterix-feature/completed-change
git branch -d feature/completed-change
```

### 3. Backup Before Removal

```bash
# Before removing a worktree, ensure it's pushed
cd ../asterix-feature/add-go-bindings
git push -u origin feature/add-go-bindings

# Verify on GitHub
gh pr view feature/add-go-bindings

# Then safe to remove worktree
cd /home/user/asterix
git worktree remove ../asterix-feature/add-go-bindings
```

### 4. Disk Space Management

Worktrees share the `.git` directory, so they're space-efficient for the git history. However:

```bash
# Each worktree has its own build artifacts
../asterix-feature/add-go-bindings/build/       # ~500 MB
../asterix-feature/wireshark-phase3/build/      # ~500 MB
../asterix-feature/improve-performance/build/   # ~500 MB

# Clean up build artifacts when not actively developing
cd ../asterix-feature/add-go-bindings
rm -rf build/ build-*/
```

---

## Troubleshooting

### Problem: Worktree Won't Create (Branch Exists)

```bash
# Error: fatal: 'feature/add-go-bindings' is already checked out at '.../add-go-bindings'

# Solution: Remove the stale worktree reference
git worktree prune

# Or use a different branch name
git worktree add ../asterix-feature/add-go-bindings-v2 -b feature/add-go-bindings-v2
```

### Problem: OpenSpec Commands Don't Work in Worktree

```bash
# The .claude/ directory is gitignored, so each worktree needs its own

# Solution 1: Run openspec init in each worktree
cd ../asterix-feature/add-go-bindings
export OPENSPEC_TELEMETRY=0
openspec init --tools claude --force

# Solution 2: Symlink .claude/ from main repo (not recommended - can cause conflicts)
cd ../asterix-feature/add-go-bindings
ln -s /home/user/asterix/.claude .claude
```

### Problem: Can't Delete Worktree (Uncommitted Changes)

```bash
# Error: fatal: 'remove' cannot be used with uncommitted changes in the worktree

# Solution: Force remove (careful - loses changes!)
git worktree remove --force ../asterix-feature/add-go-bindings

# Or commit/stash changes first
cd ../asterix-feature/add-go-bindings
git add .
git commit -m "WIP: Saving work"
cd /home/user/asterix
git worktree remove ../asterix-feature/add-go-bindings
```

---

## Automation Scripts

### Create Worktree for OpenSpec Change

```bash
# Save as: .local/scripts/worktree-new.sh
#!/bin/bash
set -e

CHANGE_NAME=$1
if [ -z "$CHANGE_NAME" ]; then
  echo "Usage: $0 <change-name>"
  exit 1
fi

WORKTREE_DIR="../asterix-feature/$CHANGE_NAME"
BRANCH_NAME="feature/$CHANGE_NAME"

echo "Creating worktree for: $CHANGE_NAME"
git worktree add "$WORKTREE_DIR" -b "$BRANCH_NAME"

echo "Setting up OpenSpec in worktree..."
cd "$WORKTREE_DIR"
export OPENSPEC_TELEMETRY=0
openspec init --tools claude --force

echo "✓ Worktree created: $WORKTREE_DIR"
echo "✓ Branch: $BRANCH_NAME"
echo ""
echo "Next steps:"
echo "  cd $WORKTREE_DIR"
echo "  # Use /opsx:new $CHANGE_NAME in Claude Code"
```

### Clean Up Merged Worktrees

```bash
# Save as: .local/scripts/worktree-cleanup.sh
#!/bin/bash
set -e

echo "Cleaning up merged worktrees..."

# List all worktrees except main
git worktree list --porcelain | grep -A1 "^worktree" | grep -v "$(git rev-parse --show-toplevel)" > /tmp/worktrees.txt

while IFS= read -r line; do
  if [[ $line == worktree* ]]; then
    WORKTREE_PATH=$(echo "$line" | cut -d' ' -f2)
    cd "$WORKTREE_PATH"
    BRANCH=$(git rev-parse --abbrev-ref HEAD)

    # Check if branch is merged into main
    if git merge-base --is-ancestor HEAD origin/main 2>/dev/null; then
      echo "✓ Removing merged worktree: $WORKTREE_PATH ($BRANCH)"
      cd -
      git worktree remove "$WORKTREE_PATH"
      git branch -d "$BRANCH" 2>/dev/null || true
    fi
  fi
done < /tmp/worktrees.txt

rm /tmp/worktrees.txt
echo "✓ Cleanup complete"
```

---

## Summary

**Git worktrees** are perfect for OpenSpec workflows:
- ✅ Work on multiple changes simultaneously
- ✅ No context switching or stashing
- ✅ Clean separation of features
- ✅ Fast testing and comparison

**Workflow:**
1. `git worktree add` - Create worktree for new change
2. `/opsx:new` - Start OpenSpec change in worktree
3. `/opsx:ff` → `/opsx:apply` - Implement in worktree
4. `git commit` → `git push` - Push from worktree
5. `git worktree remove` - Clean up when merged

**Key Commands:**
- `git worktree add <path> -b <branch>` - Create worktree
- `git worktree list` - List all worktrees
- `git worktree remove <path>` - Remove worktree
- `git worktree prune` - Clean up stale references

---

## Resources

- Git Worktree Documentation: https://git-scm.com/docs/git-worktree
- OpenSpec Documentation: https://github.com/Fission-AI/OpenSpec
- OpenSpec Workflows: https://github.com/Fission-AI/OpenSpec/blob/main/docs/workflows.md
