# How to Enable Documentation Publishing

This guide walks you through enabling ReadTheDocs (Python docs) and GitHub Pages (C++ docs) for the ASTERIX decoder project.

---

## Part 1: Enable ReadTheDocs (Python Documentation)

ReadTheDocs will automatically build and host your Python Sphinx documentation.

### Step 1: Create ReadTheDocs Account

1. **Visit ReadTheDocs:**
   - Go to https://readthedocs.org/

2. **Sign Up/Sign In:**
   - Click "Sign Up" (top right)
   - Choose "Sign in with GitHub"
   - Authorize ReadTheDocs to access your GitHub account
   - This is required for automatic builds

### Step 2: Import Your Repository

1. **Import a Project:**
   - Once logged in, you'll see your dashboard
   - Click the green "Import a Project" button
   - Or go to: https://readthedocs.org/dashboard/import/

2. **Find Your Repository:**
   - You should see a list of your GitHub repositories
   - Look for `montge/asterix`
   - If you don't see it, click "Import Manually" (see manual import below)

3. **Click the "+" button** next to `asterix` to import it

### Step 3: Configure the Project

ReadTheDocs will take you to the project configuration page:

1. **Project Details:**
   - **Name:** `asterix-decoder` (or your preferred name)
   - **Repository URL:** `https://github.com/montge/asterix`
   - **Repository type:** Git
   - **Default branch:** `master`
   - **Default version:** `latest`
   - **Programming Language:** Python

2. **Click "Next"** or "Finish"

3. **Wait for First Build:**
   - ReadTheDocs will automatically trigger a build
   - This takes about 2-5 minutes
   - You'll see a build status page

### Step 4: Verify Build Success

1. **Check Build Status:**
   - Go to your project page: `https://readthedocs.org/projects/asterix-decoder/`
   - Click on "Builds" in the left sidebar
   - You should see a green checkmark ✓ for the latest build

2. **View Documentation:**
   - If build succeeded, click "View Docs" button
   - Or visit: `https://asterix-decoder.readthedocs.io/en/latest/`
   - You should see your Sphinx documentation!

### Step 5: Add Badge to README (Optional)

Add this badge to your README.md:

```markdown
[![Documentation](https://readthedocs.org/projects/asterix-decoder/badge/?version=latest)](https://asterix-decoder.readthedocs.io/en/latest/)
```

### Troubleshooting ReadTheDocs

**Build Failed?**

1. Click on the failed build to see the error log
2. Common issues:
   - **Missing dependencies:** Check `requirements-docs.txt`
   - **Import errors:** Ensure `asterix` package builds correctly
   - **Python version:** Verify `.readthedocs.yaml` specifies Python 3.11

**Can't Find Repository?**

If your repository doesn't appear in the import list:

1. **Manual Import:**
   - Click "Import Manually" at the bottom of the import page
   - Fill in the details manually:
     - **Name:** asterix-decoder
     - **Repository URL:** https://github.com/montge/asterix
     - **Repository type:** Git
     - **Default branch:** master
   - Click "Next"

2. **Check GitHub Permissions:**
   - Go to https://github.com/settings/installations
   - Find "ReadTheDocs"
   - Make sure it has access to `montge/asterix` repository

**Documentation Not Updating?**

- Go to your project page: https://readthedocs.org/projects/asterix-decoder/
- Click "Admin" → "Integrations"
- Verify the GitHub webhook is active
- Manually trigger a build: Click "Build" button

---

## Part 2: Enable GitHub Pages (C++ Documentation)

GitHub Pages will host your Doxygen-generated C++ API documentation.

### Step 1: Enable GitHub Pages

1. **Go to Your Repository:**
   - Visit: https://github.com/montge/asterix

2. **Navigate to Settings:**
   - Click the "Settings" tab (near the top of the page)
   - If you don't see "Settings", you may need repository admin permissions

3. **Find Pages Settings:**
   - In the left sidebar, scroll down to "Code and automation" section
   - Click on "Pages"

4. **Configure Pages:**
   - **Source:** Select "GitHub Actions" from the dropdown
     - (NOT "Deploy from a branch" - that's the old method)
   - **No need to select a branch or folder**
   - Click "Save" if prompted

That's it! GitHub Pages is now enabled.

### Step 2: Trigger Doxygen Workflow

The Doxygen documentation workflow should have already triggered when you pushed the code. Let's verify:

1. **Check Actions Tab:**
   - Go to: https://github.com/montge/asterix/actions
   - Look for a workflow named "Generate C++ Documentation"
   - You should see a recent run (probably in progress or completed)

2. **If No Workflow Ran:**
   - Make a small change to trigger it:
   ```bash
   # Add a comment to any C++ file
   echo "// Documentation update" >> src/asterix/asterixformat.hxx
   git add src/asterix/asterixformat.hxx
   git commit -m "Trigger Doxygen build"
   git push origin master
   ```

3. **Monitor Build:**
   - The workflow typically takes 2-3 minutes
   - Click on the workflow run to see progress
   - Wait for green checkmark ✓

### Step 3: Access Your Documentation

Once the workflow completes successfully:

1. **Documentation URL:**
   - Visit: `https://montge.github.io/asterix/`
   - This is your C++ API documentation!

2. **It May Take a Few Minutes:**
   - GitHub Pages deployment can take 1-5 minutes after the workflow completes
   - If you get a 404, wait a bit and refresh

### Step 4: Verify Documentation

You should see:
- Doxygen-generated HTML pages
- Class hierarchy
- File list
- Search functionality
- Call graphs and diagrams

### Troubleshooting GitHub Pages

**404 Not Found?**

1. **Wait a Few Minutes:**
   - Deployment takes time after workflow completes
   - Refresh after 2-3 minutes

2. **Check Workflow Status:**
   - Go to Actions tab
   - Ensure "Generate C++ Documentation" completed successfully
   - Click on the run to see if deployment step succeeded

3. **Check Pages Settings:**
   - Go to Settings → Pages
   - Verify "Source" is set to "GitHub Actions"
   - You should see: "Your site is live at https://montge.github.io/asterix/"

4. **Check Repository Visibility:**
   - Private repos require GitHub Pro for Pages
   - Public repos (like yours) should work fine

**Workflow Failed?**

1. **Click on Failed Workflow:**
   - See which step failed
   - Common issues:
     - Doxygen not installed (should auto-install in workflow)
     - Missing source files
     - Doxyfile configuration error

2. **Check Permissions:**
   - Go to Settings → Actions → General
   - Scroll to "Workflow permissions"
   - Ensure "Read and write permissions" is selected
   - Check "Allow GitHub Actions to create and approve pull requests"
   - Save

**Old-Style Pages Enabled?**

If you previously had Pages enabled with "Deploy from branch":

1. Go to Settings → Pages
2. Change Source to "GitHub Actions"
3. Delete the `gh-pages` branch if it exists:
   ```bash
   git push origin --delete gh-pages
   ```

---

## Part 3: Update README with Documentation Links

Once both services are live, update your README.md badges:

```markdown
[![Documentation](https://readthedocs.org/projects/asterix-decoder/badge/?version=latest)](https://asterix-decoder.readthedocs.io/)
[![C++ API Docs](https://img.shields.io/badge/docs-Doxygen-blue)](https://montge.github.io/asterix/)
```

And add links in your documentation section:

```markdown
## 📖 Documentation

- **Python API Documentation:** https://asterix-decoder.readthedocs.io/
- **C++ API Documentation:** https://montge.github.io/asterix/
- **Installation Guide:** [INSTALLATION_GUIDE.md](INSTALLATION_GUIDE.md)
- **Contributing Guide:** [CONTRIBUTING.md](CONTRIBUTING.md)
```

---

## Summary: What Happens Automatically

### After Enabling ReadTheDocs:
- ✅ Builds Python documentation on every push to `master`
- ✅ Builds on every pull request (preview builds)
- ✅ Builds on every git tag (versioned docs)
- ✅ Hosts at: `https://asterix-decoder.readthedocs.io/`
- ✅ Provides PDF and EPUB downloads

### After Enabling GitHub Pages:
- ✅ Builds C++ documentation when C++ source files change
- ✅ Deploys to GitHub Pages automatically
- ✅ Hosts at: `https://montge.github.io/asterix/`
- ✅ Updates within 2-3 minutes of code changes

---

## Quick Reference

### ReadTheDocs URLs:
- **Project Dashboard:** https://readthedocs.org/projects/asterix-decoder/
- **Public Docs:** https://asterix-decoder.readthedocs.io/
- **Admin:** https://readthedocs.org/dashboard/asterix-decoder/

### GitHub Pages URLs:
- **Settings:** https://github.com/montge/asterix/settings/pages
- **Public Docs:** https://montge.github.io/asterix/
- **Workflow:** https://github.com/montge/asterix/actions/workflows/doxygen.yml

### Build Status:
- **ReadTheDocs Builds:** https://readthedocs.org/projects/asterix-decoder/builds/
- **GitHub Actions:** https://github.com/montge/asterix/actions

---

## Need Help?

- **ReadTheDocs Support:** https://docs.readthedocs.io/
- **GitHub Pages Docs:** https://docs.github.com/en/pages
- **Doxygen Manual:** https://www.doxygen.nl/manual/

If you encounter issues, check the respective build logs for detailed error messages.
