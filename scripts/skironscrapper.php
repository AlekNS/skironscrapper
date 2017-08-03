<?php


class SkironScrapperJob
{
    private $downloadUrl, $parserPath, $outputPath, $workFoler, $gribFile;

    public static function createWeakStreamContext()
    {
        return stream_context_create(['ssl' => [
            'verify_peer' => false,
            'verify_peer_name' => false,
            'allow_self_signed' => true,
        ]]);
    }

    public function __destruct ()
    {
        if (!empty($this->gribFile)) {
            @unlink($this->gribFile);
        }
    }

    public function __construct ($downloadUrl, $parserPath, $outputPath, $workFoler)
    {
        $this->downloadUrl = $downloadUrl;
        $this->parserPath = $parserPath;
        $this->outputPath = $outputPath;
        $this->workFoler = $workFoler;
    }

    private function downloadAndExtractFile()
    {
        try {
            $streamIn = fopen(
                'compress.bzip2://' . $this->downloadUrl,
                'rb'
            );

            $streamOut = fopen(
                $outFile = $this->workFoler . '/' . uniqid() . uniqid() . '.grb',
                'wb'
            );

            stream_copy_to_stream($streamIn, $streamOut);

            fclose($streamOut);
            fclose($streamIn);
        } catch (\Throwable $e) {
            if (isset($outFile)) {
                @unlink($outFile);
            }
            throw $e;
        }

        return $outFile;
    }

    private function runParser()
    {
        $handler = popen(
            sprintf(
                "\"%s\" \"%s\" \"%s\"",
                $this->parserPath,
                $this->gribFile,
                $this->outputPath
            ),
            'r'
        );
        $output = '';
        while (!feof($handler)) {
            $output .= fgets($handler);
        }
        $output = trim($output);
        if (pclose($handler) !== 0) {
            throw new \RuntimeException("Error was occurred when executing parser. " . $output);
        }
    }

    public function run()
    {
        try {
            $this->gribFile = $this->downloadAndExtractFile();
            $this->runParser($this->gribFile);
        } finally {
            @unlink($this->gribFile);
        }
    }
}


class SkironScrapperApplication
{
    private static $_instance;

    private $workPath = '/tmp';
    private $url = 'http://openskiron.org/en/openskiron';
    private $parserPath = '';
    private $output = '';
    private $concurrency = 1;

    private function __construct() { }
    private function __clone() { }

    public function getInstance()
    {
        return self::$_instance ?: self::$_instance = new SkironScrapperApplication();
    }

    public function setOptions($options)
    {
        if (is_array($options)) {
            $this->url = @$options['url'] ?: $this->url;
            $this->workPath = @$options['workPath'] ?: $this->workPath;
            $this->output = @$options['output'] ?: $this->output;
            $this->parserPath = @$options['parserPath'] ?: $this->parserPath;
            $this->concurrency = max(min(@$options['concurrency'] ?: $this->concurrency, 16), 1);
        }

        if (empty($this->output)) {
            throw new \RuntimeException('--output path not specified');
        }

        if (empty($this->parserPath)) {
            throw new \RuntimeException('--parserPath path not specified');
        }

        return $this;
    }

    private function getContent()
    {
        $content = file_get_contents($this->url, false, SkironScrapperJob::createWeakStreamContext());
        if (empty($content)) {
            throw new \RuntimeException('Content of page is empty');
        }

        return $content;
    }

    private function parseContent($content)
    {
        libxml_use_internal_errors(true);
        $dom = new DomDocument;
        $dom->loadHTML($content);
        $xpath = new DomXPath($dom);

        return array_map(
            function ($aNode) { return trim($aNode->attributes['href']->textContent); },
            iterator_to_array($xpath->query("//table[contains(@class,'easyfolderlisting')]//a[@href]"))
        );
    }

    public function run()
    {
        echo "Parse {$this->url}\n";
        $files = $this->parseContent(
            $this->getContent()
        );

        foreach($files as $file) {
            if (!preg_match(';.+/(.+)_SKIRON_WAM_.+$;i', $file, $matchSubfolder)) {
                continue;
            }
            echo "Process $file\n";
            (new SkironScrapperJob($file, $this->parserPath, $this->output . '/' . ucfirst(strtolower($matchSubfolder[1])), $this->workPath))
                ->run();
        }
        echo "Done!\n";
    }
}


SkironScrapperApplication::getInstance()
    ->setOptions(getopt('', [
            'output:',
            'concurrency:',
            'workPath::',
            'parserPath:',
            'url::',
        ]
    ))
    ->run();
