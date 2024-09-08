import os, yaml, json, struct

SRCDIR = "locale"
DESTDIR = "codegen"

basepath = os.path.dirname(os.path.realpath(__file__))
SRCDIR = basepath + "/" + SRCDIR
DESTDIR = basepath + "/" + DESTDIR

language_map = {
	"ja": "jp",
	"zh_Hant": "tw",
	"zh_Hans": "zh",
}

replace_map = {
	"a_button": "\ue000",
	"b_button": "\ue001",
	"x_button": "\ue002",
	"y_button": "\ue003",
	"l_button": "\ue004",
	"r_button": "\ue005",
	"d_pad": "\ue006",
	"playcoin": "\ue075",
}

def l(s):
	return (language_map[s] if s in language_map else s).upper()

def _s(s):
	for r, p in replace_map.items():
		s = s.replace(f"{{{r}}}", p)
	return s

translations = {}

for file in os.listdir(SRCDIR):
	if file.endswith(".yaml"):
		language = file[:-5]
		with open(SRCDIR + "/" + file, "r", encoding="utf-8") as f:
			translations[language] = yaml.safe_load(f)

headerfile = "#pragma once\n\n#include <3ds.h>\n"

outfile = "#include <3ds.h>\n#include \"lang_strings.h\"\n";

lang_keys = list(translations.keys())
lang_keys.sort(key=lambda x: 0 if x == "en" else struct.unpack(">H", x.encode("utf-8")[:2])[0])


lang_i = 0
for lang in lang_keys:
	if l(lang) not in ("JP", "EN", "FR", "DE", "IT", "ES", "ZH", "KO", "NL", "PT", "RU", "TW"):
		lang_i += 1
		headerfile += f"#define CFG_LANGUAGE_{l(lang)} {20+lang_i}\n"
headerfile += f"#define NUM_LANGUAGES {len(lang_keys)}\n"

headerfile += """
typedef const struct {
	const CFG_Language language;
	const char* text;
} LanguageString[NUM_LANGUAGES];

extern const int all_languages[];
extern const char* all_languages_str[];
"""

outfile += f"const int all_languages[{len(lang_keys)}] = {{"
for lang in lang_keys:
	outfile += f"CFG_LANGUAGE_{l(lang)}, "
outfile += "};\n"

outfile += f"const char* all_languages_str[{len(lang_keys)}] = {{"
for lang in lang_keys:
	outfile += f"\"{l(lang)}\", "
outfile += "};\n"

for key in translations["en"].keys():
	outfile += f"LanguageString {key} = {{\n"
	headerfile += f"extern LanguageString {key};\n"
	for lang in lang_keys:
		s = "0"
		if key in translations[lang] and translations[lang][key] != "":
			s = json.dumps(translations[lang][key], ensure_ascii=False)
		outfile += f"\t{{CFG_LANGUAGE_{l(lang)}, {_s(s)}}},\n"
	outfile += "};\n";

os.makedirs(DESTDIR, exist_ok=True)
with open(DESTDIR + "/lang_strings.h", "w", encoding="utf-8") as f:
	f.write(headerfile)

with open(DESTDIR + "/lang_strings.c", "w", encoding="utf-8") as f:
	f.write(outfile)
