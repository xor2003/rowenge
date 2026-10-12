#include "detect.h"

#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>
#include <strings.h>

namespace rowan_shell {

// Case-insensitive component-wise path resolution -> real path + size.
// Same contract as fopen_ci (io_compat) but needs the size too, and must not
// match directories: returns false unless the final component is a file.
static bool stat_ci(const std::string& dir, const char* relpath,
                    int64_t* size_out)
{
    std::string cur = dir;
    const char* p = relpath;
    while (*p) {
        const char* slash = strchr(p, '/');
        std::string comp(p, slash ? slash - p : strlen(p));
        if (comp.empty()) return false;

        DIR* d = opendir(cur.empty() ? "." : cur.c_str());
        if (!d) return false;
        bool found = false;
        std::string real;
        struct dirent* e;
        while ((e = readdir(d))) {
            if (strcasecmp(e->d_name, comp.c_str()) == 0) { real = e->d_name; found = true; break; }
        }
        closedir(d);
        if (!found) return false;
        cur += "/" + real;
        p = slash ? slash + 1 : p + comp.size();
    }
    struct stat st;
    if (stat(cur.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) return false;
    if (size_out) *size_out = (int64_t)st.st_size;
    return true;
}

Report detect_install_with(const char* dir, const char* only_game,
                           const EditionManifest* table, int ntable)
{
    Report r;
    if (!dir || !*dir) { r.detail = "no directory supplied"; return r; }

    int best_score = -1, best_idx = -1;
    std::vector<std::string> best_missing, best_wrongsize;

    for (int i = 0; i < ntable; ++i) {
        const EditionManifest& m = table[i];
        if (only_game && strcmp(m.game, only_game) != 0) continue;

        int score = 0;
        std::vector<std::string> miss, wrong;
        for (int s = 0; s < m.nsigs; ++s) {
            int64_t sz;
            if (!stat_ci(dir, m.sigs[s].relpath, &sz)) {
                miss.push_back(m.sigs[s].relpath);
            } else if (m.sigs[s].size >= 0 && sz != m.sigs[s].size) {
                wrong.push_back(std::string(m.sigs[s].relpath) + " exp " +
                                std::to_string(m.sigs[s].size) + " got " +
                                std::to_string(sz));
                score++;                    // file exists, size wrong — partial credit
            } else {
                score++;
            }
        }

        if (miss.empty() && wrong.empty()) {
            r.matched.push_back(std::string(m.game) + "/" + m.edition + ":" +
                                std::to_string(m.nsigs));
        }
        if (score > best_score) {
            best_score = score; best_idx = i;
            best_missing = miss; best_wrongsize = wrong;
        }
    }

    // Complete matches: distinct games -> always ambiguous. Within one game
    // the row with the most signatures wins (edition rows are subset/superset
    // by design — bdg adds markers over retail); a tie between different
    // editions is a genuinely mixed install -> ambiguous.
    if (!r.matched.empty()) {
        std::string game0, ed0;
        int best_n = -1; bool tie = false, multigame = false;
        for (auto& m : r.matched) {
            auto c1 = m.find('/'), c2 = m.rfind(':');
            std::string g = m.substr(0, c1), e = m.substr(c1 + 1, c2 - c1 - 1);
            int n = atoi(m.substr(c2 + 1).c_str());
            if (!game0.empty() && g != game0) multigame = true;
            if (n > best_n) { best_n = n; game0 = g; ed0 = e; tie = false; }
            else if (n == best_n && (g != game0 || e != ed0)) tie = true;
        }
        if (multigame || tie) {
            r.verdict = Verdict::AMBIGUOUS;
            r.detail = "ambiguous install — matched multiple editions:";
            for (auto& m : r.matched) r.detail += " " + m.substr(0, m.rfind(':'));
            return r;
        }
        r.verdict = Verdict::OK;
        r.game = game0; r.edition = ed0;
        r.detail = "detected " + game0 + "/" + ed0;
        return r;
    }
    if (best_idx >= 0 && best_score > 0) {
        r.verdict = Verdict::INCOMPLETE;
        r.missing = best_missing;
        r.wrongsize = best_wrongsize;
        r.detail = std::string("incomplete ") + table[best_idx].game + "/" +
                   table[best_idx].edition + " install";
        if (!best_missing.empty()) {
            r.detail += " — missing:";
            for (auto& f : best_missing) r.detail += " " + f;
        }
        if (!best_wrongsize.empty()) {
            r.detail += " — wrong size:";
            for (auto& f : best_wrongsize) r.detail += " " + f;
        }
        return r;
    }
    r.detail = "directory is not a recognized Rowan install";
    return r;
}

Report detect_install(const char* dir, const char* only_game)
{
    int n;
    const EditionManifest* t = registry(&n);
    return detect_install_with(dir, only_game, t, n);
}

} // namespace rowan_shell
