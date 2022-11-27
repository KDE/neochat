#!/usr/bin/env php
<?php

/*
 * This file is part of the Symfony package.
 *
 * (c) Fabien Potencier <fabien@symfony.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

require __DIR__.'/vendor/autoload.php';

use Symfony\Component\Filesystem\Filesystem;
use Symfony\Component\Finder\Finder;
use Symfony\Component\VarExporter\VarExporter;

Builder::cleanTarget();
$emojisCodePoints = Builder::getEmojisCodePoints();
Builder::saveRules(Builder::buildRules($emojisCodePoints));

final class Builder
{
    private const TARGET_DIR = __DIR__.'/../src/data/emoji/';

    public static function getEmojisCodePoints(): array
    {
        $lines = file(__DIR__.'/vendor/unicode-org/cldr/tools/cldr-code/src/main/resources/org/unicode/cldr/util/data/emoji/emoji-test.txt');
        $emojisCodePoints = [];
        foreach ($lines as $line) {
            $line = trim($line);
            if (!$line || str_starts_with($line, '#')) {
				if (str_starts_with($line, '# group:')) {
					$group = ltrim(explode(':', $line)[1]);

					if ($group == "Activities") {
						$group = "Activities";
					} else if ($group == "Animals & Nature") {
						$group = "Nature";
					} else if ($group == "Component") {
						$group = "Component";
					} else if ($group == "Flags") {
						$group = "Flags";
					} else if ($group == "Food & Drink") {
						$group = "Food";
					} else if ($group == "Objects") {
						$group = "Objects";
					} else if ($group == "People & Body") {
						$group = "People";
					} else if ($group == "Smileys & Emotion") {
						$group = "Smileys";
					} else if ($group == "Symbols") {
						$group = "Symbols";
					} else if ($group == "Travel & Places") {
						$group = "Travel";
					} else {
                        echo "Group not found";
					}
				}
                continue;
            }

            // 263A FE0F    ; fully-qualified     # ☺️ E0.6 smiling face
            preg_match('{^(?<codePoints>[\w ]+) +; [\w-]+ +# (?<emoji>.+) E\d+\.\d+ ?(?<name>.+)$}Uu', $line, $matches);
            if (!$matches) {
                throw new \DomainException("Could not parse line: \"$line\".");
            }

            $codePoints = strtolower(trim($matches['codePoints']));
            $emojisCodePoints[$codePoints] = [$matches['emoji'], $group];
            // We also add a version without the "Zero Width Joiner"
            $codePoints = str_replace('200d ', '', $codePoints);
            $emojisCodePoints[$codePoints] = [$matches['emoji'], $group];
        }

		return $emojisCodePoints;
    }

    public static function buildRules(array $emojisCodePoints): Generator
    {
        $files = (new Finder())
            ->files()
            ->in([
                // __DIR__.'/vendor/unicode-org/cldr/common/annotationsDerived',
                __DIR__.'/vendor/unicode-org/cldr/common/annotations',
            ])
            ->name('*.xml')
        ;

        $ignored = [];
        $mapsByLocale = [];

        foreach ($files as $file) {
            $locale = $file->getBasename('.xml');

            $document = new DOMDocument();
            $document->loadXML(file_get_contents($file));
            $xpath = new DOMXPath($document);
            $results = $xpath->query('.//annotation[@type="tts"]');

            foreach ($results as $result) {
                $emoji = $result->getAttribute('cp');
                $name = $result->textContent;
                $parts = preg_split('//u', $emoji, -1, \PREG_SPLIT_NO_EMPTY);
                $emojiCodePoints = implode(' ', array_map('dechex', array_map('mb_ord', $parts)));
                if (!array_key_exists($emojiCodePoints, $emojisCodePoints)) {
                    $ignored[] = [
                        'locale' => $locale,
                        'emoji' => $emoji,
                        'name' => $name,
                    ];
                    continue;
                }
	
                self::testEmoji($emoji, $locale);
                $codePointsCount = mb_strlen($emoji);
                $mapsByLocale[$locale][$codePointsCount][$emoji] = [$name, $emojisCodePoints[$emojiCodePoints][1]];
            }
        }

        ksort($mapsByLocale);

        foreach ($mapsByLocale as $locale => $maps) {
            $parentLocale = $locale;

            while (false !== $i = strrpos($parentLocale, '_')) {
                $parentLocale = substr($parentLocale, 0, $i);
                $maps += $mapsByLocale[$parentLocale] ?? [];
            }

            yield strtolower($locale) => self::createRules($maps);
        }
    }

    public static function cleanTarget(): void
    {
        $fs = new Filesystem();
        $fs->remove(self::TARGET_DIR);
        $fs->mkdir(self::TARGET_DIR);
    }

    public static function saveRules(iterable $rulesByLocale): void
    {
        $firstChars = [];
        foreach ($rulesByLocale as $locale => $rules) {
            $rulesOutput = '';
            foreach ($rules as $rule => [$text, $group]) {
				$emojiSequence = '';
				$chars = mb_str_split($rule);
				foreach ($chars as $char) {
					$emojiSequence .= sprintf('\U%08X', mb_ord($char));
				}
                $rulesOutput .= '        _emojis[EmojiModel::' . $group . '].append(Emoji{QString::fromUtf8("' . $emojiSequence  . '"), QStringLiteral("' . $text . "\")});\n";
            }
			file_put_contents(self::TARGET_DIR."/$locale.cpp", "// SPDX-FileCopyrightText: None\n// SPDX-License-Identifier: LGPL-2.0-or-later\n// This file is auto-generated. All changes will be lost. See tools/README.md\n// clang-format off\n\n#include <QString>\n#include <QHash>\n#include \"../../emojimap.h\"\n\nclass ${locale}EmojiMap: public EmojiMap {\n\npublic:\n    QHash<EmojiModel::Category, QVector<Emoji>> langEmojiMap()\n    {\n        QHash<EmojiModel::Category, QVector<Emoji>> _emojis;\n" . $rulesOutput . "};\n");

            foreach ($rules as $k => $v) {
                $firstChars[$k[0]] = $k[0];
            }
        }
        sort($firstChars);

        $quickCheck = '"'.str_replace('%', '\\x', rawurlencode(implode('', $firstChars))).'"';
    }

    private static function testEmoji(string $emoji, string $locale): void
    {
        if (!\Transliterator::createFromRules("\\$emoji > test ;")) {
            throw new \RuntimeException(sprintf('Could not create transliterator for "%s" in "%s" locale. Error: "%s".', $emoji, $locale, intl_get_error_message()));
        }
    }

    private static function createRules(array $maps): array
    {
        // We must sort the maps by the number of code points, because the order really matters:
        // 🫶🏼 must be before 🫶
        krsort($maps);
        $maps = array_merge(...$maps);

        return $maps;
    }
}