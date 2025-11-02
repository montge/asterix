# GitHub Pages Verification Checklist

Use this checklist to verify that GitHub Pages deployment for Doxygen documentation is properly configured and working.

## Pre-Deployment Checks

### Repository Configuration

- [ ] **Repository exists**: https://github.com/montge/asterix
- [ ] **Repository is public** (or GitHub Pro/Enterprise with Pages enabled for private repos)
- [ ] **Doxyfile exists**: `/tmp/asterix-restored/Doxyfile`
- [ ] **Workflow file exists**: `/tmp/asterix-restored/.github/workflows/doxygen.yml`
- [ ] **requirements-docs.txt exists**: `/tmp/asterix-restored/requirements-docs.txt`

### Doxyfile Configuration

- [ ] **PROJECT_NAME** is set to "ASTERIX Decoder C++ API"
- [ ] **OUTPUT_DIRECTORY** is set to `docs/cpp`
- [ ] **HTML_OUTPUT** is set to `html`
- [ ] **GENERATE_HTML** is set to `YES`
- [ ] **Source paths** include C++ header and source files

Verify with:
```bash
grep -E "^PROJECT_NAME|^OUTPUT_DIRECTORY|^HTML_OUTPUT|^GENERATE_HTML" Doxyfile
```

### Workflow Configuration

- [ ] **Workflow name**: "Generate C++ Documentation"
- [ ] **Triggers on**: Push to master (C++ files, Doxyfile changes)
- [ ] **Triggers on**: Manual workflow dispatch
- [ ] **Permissions**: `contents: read`, `pages: write`, `id-token: write`
- [ ] **Jobs**: `build-docs` and `deploy`
- [ ] **Artifact upload**: Uses `actions/upload-pages-artifact@v4`
- [ ] **Deployment**: Uses `actions/deploy-pages@v4`

Verify with:
```bash
cat .github/workflows/doxygen.yml | grep -A 3 "permissions:"
```

## GitHub Settings Verification

### Pages Settings

Navigate to: `https://github.com/montge/asterix/settings/pages`

- [ ] **GitHub Pages is enabled**
- [ ] **Source** is set to "GitHub Actions" (NOT "Deploy from a branch")
- [ ] **No branch-based deployment** is configured
- [ ] **Custom domain** (optional): Not configured / Configured correctly

### Actions Settings

Navigate to: `https://github.com/montge/asterix/settings/actions`

- [ ] **Actions are enabled** for this repository
- [ ] **Workflow permissions** set to "Read and write permissions" OR
- [ ] **Workflow permissions** allow Pages deployment explicitly

### Environments

Navigate to: `https://github.com/montge/asterix/settings/environments`

- [ ] **github-pages environment** exists (created automatically after first deployment)
- [ ] **No deployment branch restrictions** OR master branch is allowed

## Workflow Execution Verification

### Trigger Workflow

Navigate to: `https://github.com/montge/asterix/actions/workflows/doxygen.yml`

- [ ] **Workflow is visible** in Actions tab
- [ ] **Run workflow manually** (if first run):
  1. Click "Run workflow" button
  2. Select branch: `master`
  3. Click green "Run workflow"

### Monitor Build

- [ ] **Workflow starts** within 10 seconds
- [ ] **build-docs job**:
  - [ ] Checkout repository completes
  - [ ] Install dependencies (doxygen, graphviz) succeeds
  - [ ] Generate documentation completes
  - [ ] Create index redirect succeeds
  - [ ] Upload artifact succeeds
  - [ ] Job completes with green checkmark
- [ ] **deploy job**:
  - [ ] Waits for build-docs to complete
  - [ ] Deploy to GitHub Pages succeeds
  - [ ] Job completes with green checkmark
  - [ ] Deployment URL displayed in logs

### Check Workflow Logs

For each failed step (if any):

- [ ] **Review error messages** in workflow logs
- [ ] **Common issues**:
  - Permission errors → Check Actions settings
  - Doxygen errors → Check Doxyfile syntax
  - Upload errors → Check artifact path matches workflow
  - Deployment errors → Check Pages is enabled in Settings

## Documentation Verification

### Initial Load

Wait 1-2 minutes after deployment, then:

- [ ] **Visit**: https://montge.github.io/asterix/
- [ ] **Page loads** without errors
- [ ] **No 404 error**
- [ ] **Doxygen homepage** is displayed
- [ ] **Project name** shows "ASTERIX Decoder C++ API"

### Content Verification

- [ ] **Navigation menu** is present (Files, Classes, etc.)
- [ ] **Class list** shows C++ classes (DataBlock, Category, etc.)
- [ ] **File list** shows source files
- [ ] **Search box** is functional
- [ ] **Graphs/diagrams** display correctly (if enabled in Doxyfile)

### Cross-Reference Testing

- [ ] **Class documentation** links work
- [ ] **Method documentation** is accessible
- [ ] **File documentation** loads
- [ ] **Search results** link to correct pages
- [ ] **Source code browsing** works (if enabled)

## README Verification

### Badges

Check README.md at https://github.com/montge/asterix

- [ ] **Documentation badge** is present: `[![Documentation](https://img.shields.io/badge/docs-C%2B%2B%20API-blue.svg)](https://montge.github.io/asterix/)`
- [ ] **ReadTheDocs badge** is present: `[![ReadTheDocs](https://readthedocs.org/projects/asterix/badge/?version=latest)](https://asterix.readthedocs.io/en/latest/)`
- [ ] **Badges are clickable** and link correctly
- [ ] **Badge images load** (not broken)

### Documentation Links

- [ ] **C++ API Documentation** link exists in Documentation section
- [ ] **Link text**: "Doxygen Docs" or similar
- [ ] **Link URL**: https://montge.github.io/asterix/
- [ ] **Link works** (opens documentation)
- [ ] **No placeholder text** like "(placeholder - setup pending)"

### README.rst Verification

Check README.rst for PyPI compatibility:

- [ ] **Links to README.md** for full documentation
- [ ] **Includes basic usage examples**
- [ ] **Shows correct documentation URLs**:
  - C++ API: https://montge.github.io/asterix/
  - Python API: https://asterix.readthedocs.io/en/latest/

## Automation Verification

### Auto-Update on Code Changes

1. **Make a test change**:
   ```bash
   # Add a Doxygen comment to a C++ header
   echo "/// @brief Test documentation update" >> src/asterix/DataBlock.h
   git add src/asterix/DataBlock.h
   git commit -m "Test: Verify Doxygen auto-update"
   git push origin master
   ```

2. **Verify workflow triggers**:
   - [ ] **Workflow starts** automatically within 10 seconds
   - [ ] **Build completes** successfully
   - [ ] **Deployment completes** successfully

3. **Check documentation updated**:
   - [ ] **Wait 2-3 minutes** for CDN propagation
   - [ ] **Hard refresh** (Ctrl+Shift+R) documentation page
   - [ ] **Test comment appears** in updated documentation
   - [ ] **Commit hash** in footer matches latest commit (if shown)

### Auto-Update on Doxyfile Changes

1. **Modify Doxyfile**:
   ```bash
   # Change project name or version
   sed -i 's/PROJECT_NAME.*=.*/PROJECT_NAME = "ASTERIX Decoder C++ API v2.8.10"/' Doxyfile
   git add Doxyfile
   git commit -m "Test: Update Doxyfile"
   git push origin master
   ```

2. **Verify workflow triggers**:
   - [ ] **Workflow starts** automatically
   - [ ] **Build uses new settings**
   - [ ] **Updated project name** appears in documentation

## Performance Verification

### Build Time

- [ ] **Build job** completes in under 5 minutes
- [ ] **Deploy job** completes in under 2 minutes
- [ ] **Total workflow time** is under 10 minutes

### Page Load Speed

- [ ] **Initial page load** is under 3 seconds
- [ ] **Subsequent page loads** are under 1 second (cached)
- [ ] **Search functionality** responds quickly

## Security Verification

### HTTPS

- [ ] **Documentation URL** uses HTTPS: https://montge.github.io/asterix/
- [ ] **No mixed content warnings** in browser console
- [ ] **Valid SSL certificate**

### Permissions

- [ ] **Workflow permissions** are minimal (read contents, write pages)
- [ ] **No unnecessary secrets** configured
- [ ] **No sensitive data** in documentation

## Maintenance Verification

### Workflow Reliability

Check last 5 workflow runs at: `https://github.com/montge/asterix/actions/workflows/doxygen.yml`

- [ ] **Success rate**: 80%+ (4+ out of 5 successful)
- [ ] **No persistent failures**
- [ ] **Average build time** is consistent
- [ ] **No timeout issues**

### Documentation Freshness

- [ ] **Documentation version** matches current code
- [ ] **Deprecated features** are marked
- [ ] **New features** are documented
- [ ] **Commit information** shown (if configured)

## Troubleshooting Checklist

If any checks fail, consult:

- [ ] **GITHUB_PAGES_SETUP.md** - Detailed setup instructions
- [ ] **Workflow logs** - Specific error messages
- [ ] **GitHub Status** - https://www.githubstatus.com/
- [ ] **GitHub Pages docs** - https://docs.github.com/en/pages

## Common Issues and Solutions

### Issue: 404 on documentation URL

**Check**:
- [ ] Repository is public
- [ ] Pages source is "GitHub Actions"
- [ ] Workflow completed both jobs
- [ ] Waited 2+ minutes after deployment

**Solution**: See "404 Error on documentation URL" in GITHUB_PAGES_SETUP.md

### Issue: Documentation not updating

**Check**:
- [ ] Workflow was triggered (check Actions tab)
- [ ] Changes match workflow path filters
- [ ] CDN cache has refreshed (wait 2-3 minutes)

**Solution**: See "Documentation is outdated" in GITHUB_PAGES_SETUP.md

### Issue: Workflow permission errors

**Check**:
- [ ] Workflow permissions in Settings → Actions
- [ ] Pages deployment permission granted

**Solution**: See "Workflow fails with permission error" in GITHUB_PAGES_SETUP.md

## Final Verification

- [ ] **All pre-deployment checks** passed
- [ ] **All GitHub settings** verified
- [ ] **Workflow execution** successful
- [ ] **Documentation accessible** and correct
- [ ] **README badges and links** working
- [ ] **Automation** verified (auto-updates work)
- [ ] **Performance** acceptable
- [ ] **Security** verified
- [ ] **No outstanding issues** or errors

## Sign-off

Date: _______________

Verified by: _______________

Notes:
```
_________________________________________________________________
_________________________________________________________________
_________________________________________________________________
```

---

**Checklist Version**: 1.0
**Last Updated**: 2025-11-01
**Documentation URL**: https://montge.github.io/asterix/
