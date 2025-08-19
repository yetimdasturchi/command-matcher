<?php

namespace Uz\Manu\Command\Matcher;

use FFI;

class CommandMatcher
{
    private FFI $ffi;

    public function __construct(string $libPath = __DIR__ . '/out/libmanulu.so')
    {
        $this->ffi = FFI::cdef(<<<'CDEF'
            int init_res(const char* base_path);
            int get_from_text(const char* input);

            const char* get_domain(void);
            const char* get_operational_label(void);
            const char* get_original(void);
            const char* get_answer(void);
            const char* get_json(void);
        CDEF, $libPath);
    }

    public function initRes(string $dir): int
    {
        return $this->ffi->init_res($dir);
    }

    public function getFromText(string $input): int
    {
        return $this->ffi->get_from_text($input);
    }

    public function getDomain(): string
    {
        return self::cstr($this->ffi->get_domain());
    }

    public function getOperationalLabel(): string
    {
        return self::cstr($this->ffi->get_operational_label());
    }

    public function getIndex(): string
    {
        return self::cstr($this->ffi->get_original());
    }

    public function getAnswer(): string
    {
        return self::cstr($this->ffi->get_answer());
    }

    public function getJson(): string
    {
        return self::cstr($this->ffi->get_json());
    }

    private static function cstr($val): string
    {
        return ($val instanceof \FFI\CData) ? FFI::string($val) : (string)$val;
    }
}

if (PHP_SAPI === 'cli' && basename(__FILE__) === basename($_SERVER['argv'][0] ?? '')) {
    if (!filter_var(ini_get('ffi.enable'), FILTER_VALIDATE_BOOL)) {
        fwrite(STDERR, "FFI is disabled. Run with: php -d ffi.enable=1 " . basename(__FILE__) . PHP_EOL);
        exit(1);
    }

    $cm = new CommandMatcher(__DIR__ . '/out/libmanulu.so');

    $rc = $cm->initRes(__DIR__ . '/static/');
    if ($rc !== 0) {
        fwrite(STDERR, "Failed to init!" . PHP_EOL);
        exit(1);
    }

    $rc = $cm->getFromText("chiroqni yoq");
    if ($rc !== 0) {
        fwrite(STDERR, "Processing failed!" . PHP_EOL);
        exit(1);
    }

    echo "Domain: " . $cm->getDomain() . PHP_EOL;
    echo "Label: "  . $cm->getOperationalLabel() . PHP_EOL;
    echo "Index: "  . $cm->getIndex() . PHP_EOL;
    echo "Answer: " . $cm->getAnswer() . PHP_EOL;
    echo "Json: "   . $cm->getJson() . PHP_EOL;
}
