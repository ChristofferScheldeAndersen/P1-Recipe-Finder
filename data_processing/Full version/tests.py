"""python3 tests.py (Linux) eller py tests.py (Windows); kræver GCC i PATH."""
import json
import os
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent
REMOVED = {'52772', '52774', '52829', '52879', '52933', '52935', '53017',
           '53057', '53067', '53083', '53093', '53116', '53308', '53336',
           '53342', '53454', '53490', '53494', '53535'}


class RecipeTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.build = tempfile.TemporaryDirectory()
        cls.binary = Path(cls.build.name) / ('opskrifter.exe' if os.name == 'nt' else 'opskrifter')
        subprocess.run([os.environ.get('CC', 'gcc'), '-std=c11', '-Wall', '-Wextra', '-Wpedantic',
                        str(ROOT / 'opskrifter.c'), str(ROOT / 'cJSON.c'), '-I' + str(ROOT),
                        '-lm', '-o', str(cls.binary)], check=True)

    @classmethod
    def tearDownClass(cls):
        cls.build.cleanup()

    def run_program(self, source):
        with tempfile.TemporaryDirectory() as d:
            folder = Path(d)
            input_file = folder / 'opskrifter.json'
            input_file.write_text(source, encoding='utf-8')
            original_bytes = input_file.read_bytes()
            result = subprocess.run([str(self.binary)], cwd=folder, capture_output=True,
                                    text=True, encoding='utf-8')
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(input_file.read_bytes(), original_bytes)
            self.assertIn('Before:', result.stdout)
            self.assertIn('0.5-0.75 tsp', result.stdout)
            self.assertEqual(result.stdout.count('-> amount:'), 22)
            self.assertFalse((folder / 'opskrifter_opdelt.json.tmp').exists())
            self.assertEqual({p.name for p in folder.iterdir()},
                             {'opskrifter.json', 'opskrifter_opdelt.json'})
            return json.loads((folder / 'opskrifter_opdelt.json').read_text(encoding='utf-8'))

    def convert(self, raw, instructions=''):
        meal = {'idMeal': '99999', 'strMeasure1': raw, 'strInstructions': instructions}
        return self.run_program(json.dumps({'meals': [meal]}, ensure_ascii=False))['meals'][0]

    def test_original_793_to_774_and_full_output(self):
        source = (ROOT / 'opskrifter.json').read_text(encoding='utf-8')
        data = json.loads(source)
        self.assertEqual(len(data['meals']), 793)
        self.assertEqual({m['idMeal'] for m in data['meals']} & REMOVED, REMOVED)
        actual = self.run_program(source)
        self.assertEqual(len(actual['meals']), 774)
        self.assertFalse({m['idMeal'] for m in actual['meals']} & REMOVED)
        self.assertEqual([m['idMeal'] for m in actual['meals']],
                         [m['idMeal'] for m in data['meals'] if m['idMeal'] not in REMOVED])
        expected = json.loads((ROOT / 'tests/forventet_output.json').read_text(encoding='utf-8'))
        self.assertEqual(actual, expected)

    def test_adjacent_ids_duplicates_and_exact_matching(self):
        ids = ['99999', *sorted(REMOVED), '52772', '5277', '527720', '12345']
        data = {'meals': [{'idMeal': x, 'strMeasure1': '400g'} for x in ids]}
        actual = self.run_program(json.dumps(data))
        self.assertEqual([m['idMeal'] for m in actual['meals']], ['99999', '5277', '527720', '12345'])
        self.assertTrue(all(m['portions'] == 1 for m in actual['meals']))

    def test_already_filtered_input(self):
        data = {'meals': [{'idMeal': '99999', 'strMeasure1': '2 tbsp chopped'}]}
        first = self.run_program(json.dumps(data))
        second = self.run_program(json.dumps(first))
        self.assertEqual(first, second)

    def test_all_removed_and_empty(self):
        for meals in [[], [{'idMeal': x} for x in sorted(REMOVED)]]:
            with self.subTest(meals=len(meals)):
                self.assertEqual(self.run_program(json.dumps({'meals': meals})), {'meals': []})

    def test_quantities_rounding_and_notes(self):
        cases = [('2 cloves', 2, 'cloves'), ('3 400g Cans', 1200, 'g'),
                 ('0.5-0.75 tsp', .6, 'teaspoons'), ('2-1/2 cups', 600, 'ml'),
                 ('1 ½ cups', 360, 'ml'), ('1 pint', 473, 'ml'), ('1 quart', 946, 'ml'),
                 ('1 fl oz', 30, 'ml'), ('12.5 g', 13, 'g'), ('24.6 ml', 25, 'ml'),
                 ('1.25 kg', 1.3, 'kg')]
        for raw, amount, unit in cases:
            with self.subTest(raw=raw):
                meal = self.convert(raw)
                self.assertEqual(meal['strMeasure1amount'], amount)
                self.assertEqual(meal['strMeasure1unit'], unit)
                self.assertEqual(meal['strMeasure1'], raw)
        meal = self.convert('½ tsp dissolved in ½ cup milk')
        self.assertEqual(meal['strMeasure1note'], 'dissolved in 120 ml milk')

    def test_instructions_and_portions(self):
        meal = self.convert('3 400g Cans', 'Use 1 oz tomato. Cut ⅛-inch pieces. Cook 20 minutes at 180 C.')
        self.assertEqual(meal['portions'], 3)
        self.assertEqual(meal['strInstructions'],
                         'Use 28 g tomato. Cut 0.3 cm pieces. Cook 20 minutes at 180 C.')
        for raw, expected in [('600g', 2), ('1kg', 3), ('200g', 1), ('2 cloves', 0)]:
            with self.subTest(raw=raw):
                self.assertEqual(self.convert(raw)['portions'], expected)

    def test_json_format_and_escaping(self):
        raw = '2 tbsp chopped "fine"\nwith \\ and æøå'
        data = {'meals': [{'idMeal': '99999', 'strMeasure1': raw}]}
        for indent in [None, 2]:
            for ascii_only in [True, False]:
                with self.subTest(indent=indent, ascii_only=ascii_only):
                    meal = self.run_program(json.dumps(data, indent=indent, ensure_ascii=ascii_only))['meals'][0]
                    self.assertEqual(meal['strMeasure1'], raw)
                    self.assertEqual(meal['strMeasure1note'], raw[len('2 tbsp '):])

    def test_invalid_input_preserves_existing_output(self):
        for source in ['{"meals":[}', '{}', '{"meals":null}']:
            with self.subTest(source=source), tempfile.TemporaryDirectory() as d:
                folder = Path(d)
                (folder / 'opskrifter.json').write_text(source, encoding='utf-8')
                output = folder / 'opskrifter_opdelt.json'
                output.write_text('previous-output', encoding='utf-8')
                result = subprocess.run([str(self.binary)], cwd=folder, capture_output=True)
                self.assertNotEqual(result.returncode, 0)
                self.assertEqual(output.read_text(encoding='utf-8'), 'previous-output')
                self.assertFalse((folder / 'opskrifter_opdelt.json.tmp').exists())


if __name__ == '__main__':
    unittest.main(verbosity=2)
