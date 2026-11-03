package forms

import (
	"fmt"
	"strings"

	"github.com/ipoluianov/nui/ui"
	"github.com/ipoluianov/nui/ui/i18n"
)

// Strings are all the texts of the IDE. A misspelled field is a compile
// error; a field a language leaves empty falls back to English
// (strings_test.go checks that none is left).
type Strings struct {
	Error       string
	Description string // under the name: on the start page, in About

	// Main menu
	MenuFile  string
	MenuEdit  string
	MenuView  string
	MenuBuild string
	MenuHelp  string

	OpenFolder    string
	OpenRecent    string
	NoRecent      string
	ClearRecent   string
	NewFile       string
	NewFolder     string
	Save          string
	SaveAll       string
	CloseTab      string
	CloseFolder   string
	Exit          string
	Undo          string
	Redo          string
	Cut           string
	Copy          string
	Paste         string
	SelectAll     string
	Find          string
	FindNext      string
	FindPrevious  string
	GoToLine      string
	Theme         string
	ThemeDark     string
	ThemeLight    string
	Language      string
	SystemDefault string
	ZoomIn        string
	ZoomOut       string
	ZoomReset     string
	Build         string
	Run           string
	Stop          string
	Settings      string
	About         string
	Website       string

	// Project tree and tabs
	Rename      string
	Delete      string
	Refresh     string
	CopyPath    string
	CloseOthers string
	CloseAll    string

	NewFileTitle    string
	NewFolderTitle  string
	NamePrompt      func(dir string) string
	RenameTitle     string
	RenamePrompt    string
	NameEmpty       string
	NameInvalid     string
	AlreadyExists   func(name string) string
	DeleteTitle     string
	DeleteFileAsk   func(name string) string
	DeleteFolderAsk func(name string) string

	// Unsaved changes, files
	UnsavedTitle   string
	SaveChangesTo  func(name string) string
	SaveChangesN   func(n int) string
	CannotOpen     func(path, err string) string
	NotText        func(name string) string
	TooLarge       func(name string) string
	CannotSave     func(name, err string) string
	ChangedOnDisk  string
	ReloadAsk      func(name string) string
	FolderNotFound func(dir string) string
	CannotLoad     func(dir, err string) string

	// Start page, the empty editor
	StartOpen     string
	StartRecent   string
	StartNoRecent string
	StartHint     string
	NoOpenFiles   string
	EditorHints   func(open, build, run string) string

	// Find bar, go to line
	FindLabel  string
	MatchCase  string
	Previous   string
	Next       string
	Matches    func(n int) string
	NoMatches  string
	GoToTitle  string
	GoToPrompt func(lines int) string
	GoToBad    string

	// Output panel, build and run
	OutputTab      string
	ProblemsTab    string
	ProblemsN      func(n int) string
	ClearOutput    string
	NoProblems     string
	Building       func(name string) string
	BuildOK        func(duration string) string
	BuildFailed    func(errors, warnings int) string
	BuildCancelled string
	Running        func(cmd string) string
	Exited         func(code int, duration string) string
	Stopped        string
	CannotRun      func(err string) string
	Ready          string
	StatusBuilding string
	StatusRunning  string
	LineCol        func(line, col int) string

	// Project settings
	SettingsTitle  string
	Name           string
	NameHint       string
	Standard       string
	StdDefault     string
	Output         string
	Sources        string
	SourcesHint    string
	IncludeDirs    string
	Defines        string
	DefinesHint    string
	Libs           string
	LibsHint       string
	LibDirs        string
	CFlags         string
	CFlagsHint     string
	Args           string
	ArgsHint       string
	OnePerLine     string
	SettingsSaved  string
	CannotSaveProj func(err string) string

	// About
	AboutTitle   func(name string) string
	Version      string
	Author       string
	License      string
	VisitWebsite string
	Close        string
}

// languages are offered in the View menu; the names stay in their own language
var languages = []struct{ tag, name string }{
	{"en", "English"},
	{"ru", "Русский"},
}

func plural(lang string, n int, forms ...string) string {
	return fmt.Sprint(n, " ", i18n.Plural(lang, n, forms...))
}

var en = Strings{
	Error:       "Error",
	Description: "A C compiler and a simple IDE",

	MenuFile:  "&File",
	MenuEdit:  "&Edit",
	MenuView:  "&View",
	MenuBuild: "&Build",
	MenuHelp:  "&Help",

	OpenFolder:    "&Open Folder...",
	OpenRecent:    "Open &Recent",
	NoRecent:      "No recent folders",
	ClearRecent:   "&Clear Recent",
	NewFile:       "&New File...",
	NewFolder:     "New Fol&der...",
	Save:          "&Save",
	SaveAll:       "Save A&ll",
	CloseTab:      "&Close Tab",
	CloseFolder:   "Close &Folder",
	Exit:          "E&xit",
	Undo:          "&Undo",
	Redo:          "&Redo",
	Cut:           "Cu&t",
	Copy:          "&Copy",
	Paste:         "&Paste",
	SelectAll:     "Select &All",
	Find:          "&Find...",
	FindNext:      "Find &Next",
	FindPrevious:  "Find &Previous",
	GoToLine:      "&Go to Line...",
	Theme:         "&Theme",
	ThemeDark:     "&Dark",
	ThemeLight:    "&Light",
	Language:      "&Language",
	SystemDefault: "System default",
	ZoomIn:        "Zoom &In",
	ZoomOut:       "Zoom &Out",
	ZoomReset:     "&Reset Zoom",
	Build:         "&Build",
	Run:           "&Run",
	Stop:          "S&top",
	Settings:      "Project &Settings...",
	About:         "&About AltC",
	Website:       "AltC &Website",

	Rename:      "&Rename...",
	Delete:      "&Delete",
	Refresh:     "Re&fresh",
	CopyPath:    "Copy &Path",
	CloseOthers: "Close &Others",
	CloseAll:    "Close &All",

	NewFileTitle:    "New File",
	NewFolderTitle:  "New Folder",
	NamePrompt:      func(dir string) string { return "Name in " + dir + ":" },
	RenameTitle:     "Rename",
	RenamePrompt:    "New name:",
	NameEmpty:       "Enter a name",
	NameInvalid:     "The name can't be \".\" or \"..\" or contain \"\\\"",
	AlreadyExists:   func(name string) string { return name + " already exists" },
	DeleteTitle:     "Delete",
	DeleteFileAsk:   func(name string) string { return "Delete the file " + name + "?" },
	DeleteFolderAsk: func(name string) string { return "Delete the folder " + name + " with everything in it?" },

	UnsavedTitle:  "Unsaved changes",
	SaveChangesTo: func(name string) string { return "Save the changes to " + name + "?" },
	SaveChangesN:  func(n int) string { return "Save the changes to " + plural("en", n, "file", "files") + "?" },
	CannotOpen:    func(path, err string) string { return "Cannot open " + path + ":\n\n" + err },
	NotText:       func(name string) string { return name + " is not a text file" },
	TooLarge:      func(name string) string { return name + " is too large to edit" },
	CannotSave:    func(name, err string) string { return "Cannot save " + name + ":\n\n" + err },
	ChangedOnDisk: "File changed",
	ReloadAsk: func(name string) string {
		return name + " has changed on disk. Reload it and lose the changes made here?"
	},
	FolderNotFound: func(dir string) string { return "The folder " + dir + " no longer exists" },
	CannotLoad:     func(dir, err string) string { return "Cannot open the project " + dir + ":\n\n" + err },

	StartOpen:     "Open Folder...",
	StartRecent:   "Recent",
	StartNoRecent: "No recent folders yet",
	StartHint:     "Open a folder with C sources: every .c file in it is built into one program.\nThe optional altc.json in the folder sets the name, the defines, the libraries.",
	NoOpenFiles:   "No open files",
	EditorHints: func(open, build, run string) string {
		return "Double-click a file in the tree to open it\n\nOpen a folder: " + open + "\nBuild: " + build + "\nBuild and run: " + run
	},

	FindLabel:  "Find:",
	MatchCase:  "Match case",
	Previous:   "Previous",
	Next:       "Next",
	Matches:    func(n int) string { return plural("en", n, "match", "matches") },
	NoMatches:  "No matches",
	GoToTitle:  "Go to Line",
	GoToPrompt: func(lines int) string { return fmt.Sprintf("Line (1 - %d), or line:column:", lines) },
	GoToBad:    "Enter a line number",

	OutputTab:   "Output",
	ProblemsTab: "Problems",
	ProblemsN:   func(n int) string { return fmt.Sprintf("Problems (%d)", n) },
	ClearOutput: "Clear",
	NoProblems:  "No problems",
	Building:    func(name string) string { return "Building " + name + "..." },
	BuildOK:     func(d string) string { return "Build succeeded in " + d },
	BuildFailed: func(e, w int) string {
		return "Build failed: " + plural("en", e, "error", "errors") + ", " + plural("en", w, "warning", "warnings")
	},
	BuildCancelled: "Build cancelled",
	Running:        func(cmd string) string { return "Running " + cmd },
	Exited:         func(code int, d string) string { return fmt.Sprintf("Process exited with code %d in %s", code, d) },
	Stopped:        "Process stopped",
	CannotRun:      func(err string) string { return "Cannot run the program: " + err },
	Ready:          "Ready",
	StatusBuilding: "Building...",
	StatusRunning:  "Running...",
	LineCol:        func(line, col int) string { return fmt.Sprintf("Ln %d, Col %d", line, col) },

	SettingsTitle:  "Project Settings",
	Name:           "Name:",
	NameHint:       "the name of the folder",
	Standard:       "Standard:",
	StdDefault:     "Default (c17)",
	Output:         "Output:",
	Sources:        "Sources:",
	SourcesHint:    "files or folders; empty: every .c file",
	IncludeDirs:    "Include folders:",
	Defines:        "Defines:",
	DefinesHint:    "NAME or NAME=VALUE",
	Libs:           "Libraries:",
	LibsHint:       "m, pthread...",
	LibDirs:        "Library folders:",
	CFlags:         "Extra flags:",
	CFlagsHint:     "e.g. -O2 -w",
	Args:           "Run arguments:",
	ArgsHint:       "the arguments of the program on Run",
	OnePerLine:     "One per line",
	SettingsSaved:  "Saved to altc.json",
	CannotSaveProj: func(err string) string { return "Cannot save altc.json:\n\n" + err },

	AboutTitle:   func(name string) string { return "About " + name },
	Version:      "Version",
	Author:       "Author",
	License:      "License",
	VisitWebsite: "Visit Website",
	Close:        "Close",
}

var ru = Strings{
	Error:       "Ошибка",
	Description: "Компилятор C и простая среда разработки",

	MenuFile:  "&Файл",
	MenuEdit:  "&Правка",
	MenuView:  "&Вид",
	MenuBuild: "&Сборка",
	MenuHelp:  "Спр&авка",

	OpenFolder:    "&Открыть папку...",
	OpenRecent:    "Открыть &недавние",
	NoRecent:      "Нет недавних папок",
	ClearRecent:   "&Очистить список",
	NewFile:       "&Новый файл...",
	NewFolder:     "Новая п&апка...",
	Save:          "&Сохранить",
	SaveAll:       "Сохранить &все",
	CloseTab:      "&Закрыть вкладку",
	CloseFolder:   "Закрыть &папку",
	Exit:          "В&ыход",
	Undo:          "&Отменить",
	Redo:          "&Повторить",
	Cut:           "&Вырезать",
	Copy:          "&Копировать",
	Paste:         "Вст&авить",
	SelectAll:     "Выделить &всё",
	Find:          "&Найти...",
	FindNext:      "Найти &далее",
	FindPrevious:  "Найти &ранее",
	GoToLine:      "Перейти к &строке...",
	Theme:         "&Тема",
	ThemeDark:     "&Тёмная",
	ThemeLight:    "&Светлая",
	Language:      "&Язык",
	SystemDefault: "Как в системе",
	ZoomIn:        "&Увеличить",
	ZoomOut:       "У&меньшить",
	ZoomReset:     "&Обычный размер",
	Build:         "&Собрать",
	Run:           "&Запустить",
	Stop:          "&Остановить",
	Settings:      "&Настройки проекта...",
	About:         "&О программе AltC",
	Website:       "&Сайт AltC",

	Rename:      "&Переименовать...",
	Delete:      "&Удалить",
	Refresh:     "&Обновить",
	CopyPath:    "Копировать &путь",
	CloseOthers: "Закрыть &другие",
	CloseAll:    "Закрыть &все",

	NewFileTitle:   "Новый файл",
	NewFolderTitle: "Новая папка",
	NamePrompt:     func(dir string) string { return "Имя в " + dir + ":" },
	RenameTitle:    "Переименовать",
	RenamePrompt:   "Новое имя:",
	NameEmpty:      "Введите имя",
	NameInvalid:    "Имя не может быть \".\" или \"..\" и содержать \"\\\"",
	AlreadyExists:  func(name string) string { return name + " уже существует" },
	DeleteTitle:    "Удаление",
	DeleteFileAsk:  func(name string) string { return "Удалить файл " + name + "?" },
	DeleteFolderAsk: func(name string) string {
		return "Удалить папку " + name + " со всем содержимым?"
	},

	UnsavedTitle:  "Несохранённые изменения",
	SaveChangesTo: func(name string) string { return "Сохранить изменения в " + name + "?" },
	SaveChangesN: func(n int) string {
		return "Сохранить изменения в " + plural("ru", n, "файле", "файлах", "файлах") + "?"
	},
	CannotOpen: func(path, err string) string { return "Не удалось открыть " + path + ":\n\n" + err },
	NotText:    func(name string) string { return name + " не текстовый файл" },
	TooLarge: func(name string) string {
		return name + " слишком большой для редактирования"
	},
	CannotSave:    func(name, err string) string { return "Не удалось сохранить " + name + ":\n\n" + err },
	ChangedOnDisk: "Файл изменён",
	ReloadAsk: func(name string) string {
		return name + " изменён на диске. Загрузить его заново, потеряв изменения, сделанные здесь?"
	},
	FolderNotFound: func(dir string) string { return "Папки " + dir + " больше нет" },
	CannotLoad: func(dir, err string) string {
		return "Не удалось открыть проект " + dir + ":\n\n" + err
	},

	StartOpen:     "Открыть папку...",
	StartRecent:   "Недавние",
	StartNoRecent: "Недавних папок пока нет",
	StartHint:     "Откройте папку с исходниками на C: все .c файлы в ней собираются в одну программу.\nНеобязательный altc.json в папке задаёт имя, определения, библиотеки.",
	NoOpenFiles:   "Нет открытых файлов",
	EditorHints: func(open, build, run string) string {
		return "Дважды щёлкните файл в дереве, чтобы открыть его\n\nОткрыть папку: " + open + "\nСобрать: " + build + "\nСобрать и запустить: " + run
	},

	FindLabel: "Найти:",
	MatchCase: "Учитывать регистр",
	Previous:  "Назад",
	Next:      "Далее",
	Matches: func(n int) string {
		return plural("ru", n, "совпадение", "совпадения", "совпадений")
	},
	NoMatches: "Нет совпадений",
	GoToTitle: "Переход к строке",
	GoToPrompt: func(lines int) string {
		return fmt.Sprintf("Строка (1 - %d) или строка:столбец:", lines)
	},
	GoToBad: "Введите номер строки",

	OutputTab:   "Вывод",
	ProblemsTab: "Проблемы",
	ProblemsN:   func(n int) string { return fmt.Sprintf("Проблемы (%d)", n) },
	ClearOutput: "Очистить",
	NoProblems:  "Проблем нет",
	Building:    func(name string) string { return "Сборка " + name + "..." },
	BuildOK:     func(d string) string { return "Собрано за " + d },
	BuildFailed: func(e, w int) string {
		return "Сборка не удалась: " + plural("ru", e, "ошибка", "ошибки", "ошибок") + ", " + plural("ru", w, "предупреждение", "предупреждения", "предупреждений")
	},
	BuildCancelled: "Сборка отменена",
	Running:        func(cmd string) string { return "Запуск " + cmd },
	Exited: func(code int, d string) string {
		return fmt.Sprintf("Процесс завершился с кодом %d за %s", code, d)
	},
	Stopped:        "Процесс остановлен",
	CannotRun:      func(err string) string { return "Не удалось запустить программу: " + err },
	Ready:          "Готово",
	StatusBuilding: "Сборка...",
	StatusRunning:  "Выполняется...",
	LineCol:        func(line, col int) string { return fmt.Sprintf("Стр %d, Стл %d", line, col) },

	SettingsTitle:  "Настройки проекта",
	Name:           "Имя:",
	NameHint:       "имя папки",
	Standard:       "Стандарт:",
	StdDefault:     "По умолчанию (c17)",
	Output:         "Результат:",
	Sources:        "Исходники:",
	SourcesHint:    "файлы или папки; пусто: все .c файлы",
	IncludeDirs:    "Папки заголовков:",
	Defines:        "Определения:",
	DefinesHint:    "ИМЯ или ИМЯ=ЗНАЧЕНИЕ",
	Libs:           "Библиотеки:",
	LibsHint:       "m, pthread...",
	LibDirs:        "Папки библиотек:",
	CFlags:         "Доп. флаги:",
	CFlagsHint:     "например, -O2 -w",
	Args:           "Аргументы запуска:",
	ArgsHint:       "аргументы программы при запуске",
	OnePerLine:     "По одному в строке",
	SettingsSaved:  "Сохранено в altc.json",
	CannotSaveProj: func(err string) string { return "Не удалось сохранить altc.json:\n\n" + err },

	AboutTitle:   func(name string) string { return "О программе " + name },
	Version:      "Версия",
	Author:       "Автор",
	License:      "Лицензия",
	VisitWebsite: "Открыть сайт",
	Close:        "Закрыть",
}

var catalog = i18n.NewCatalog(en, map[string]Strings{"ru": ru})

// T returns the texts in the language of the application
func T() *Strings {
	return catalog.Get(ui.Language())
}

// SetLanguage switches the application to the language of the settings, "" - the system's
func SetLanguage(lang string) {
	if lang == "" {
		lang = ui.SystemLanguage()
	}
	ui.SetLanguage(lang)
}

// plain drops the mnemonic marks of a menu text: "&Save" is "Save"
func plain(s string) string {
	s = strings.ReplaceAll(s, "&&", "\x00")
	s = strings.ReplaceAll(s, "&", "")
	s = strings.TrimSuffix(s, "...")
	return strings.ReplaceAll(s, "\x00", "&")
}
