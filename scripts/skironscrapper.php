<?php


class SkironScrapperJob
{
    private $downloadUrl, $parserPath, $outputPath, $workFoler, $gribFile;
    private $phpProcess;

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

    public function getName()
    {
        return $this->downloadUrl;
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

        return 0;
    }

    static public function executeAsProcess($serializedValue)
    {
        $obj = unserialize(base64_decode($serializedValue));
        return $obj->doJob();
    }

    public function isDone()
    {
        if ($this->phpProcess == null) {
            return false;
        }

        if (!is_resource($this->phpProcess)) {
            return true;
        }

        $status = proc_get_status($this->phpProcess);
        if (empty($status['running'])) {
            proc_close($this->phpProcess);
            return true;
        }

        return false;
    }

    public function isRunning()
    {
        if ($this->phpProcess == null) {
            return false;
        }

        $status = proc_get_status($this->phpProcess);
        return !empty($status['running']);
    }

    private function doJob()
    {
        try {
            $this->gribFile = $this->downloadAndExtractFile();
            return $this->runParser($this->gribFile);
        } finally {
            @unlink($this->gribFile);
        }
    }

    public function run()
    {
        global $argv;

        $serializedJob = base64_encode(serialize($this));
        $this->phpProcess = proc_open(
            sprintf(
                "php \"%s\" --job=%s",
                $argv[0],
                $serializedJob
            ),
            [],
            $pipes
        );

        if (!is_resource($this->phpProcess)) {
            throw new \RuntimeException('Cant launch job php process.');
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

        $jobs = [];

        foreach($files as $file) {
            if (!preg_match(';.+/(.+)_SKIRON_WAM_.+$;i', $file, $matchSubfolder)) {
                continue;
            }
            $jobs[] = new SkironScrapperJob($file, $this->parserPath, $this->output . '/' . ucfirst(strtolower($matchSubfolder[1])), $this->workPath);
        }

        $currentJobsCount = 0;
        echo "Jobs count is " . count($jobs) . "\n";
        while(count($jobs) > 0) {
            foreach($jobs as $job) {
                if ($job->isDone()) {
                    echo "Done {$job->getName()}.\n";
                    array_splice($jobs, array_search($job, $jobs), 1);
                    $currentJobsCount -= 1;
                    continue;
                }
                if (!$job->isRunning() && $currentJobsCount < $this->concurrency) {
                    $job->run();
                    echo "Started {$job->getName()}...\n";
                    $currentJobsCount += 1;
                }
            }

            usleep(10000);
        } 

        echo "Done!\n";
    }
}


if ($argc == 2 && strpos($argv[1], '--job=') === 0) {
    exit(SkironScrapperJob::executeAsProcess(substr($argv[1], 6)));
} else {
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
}